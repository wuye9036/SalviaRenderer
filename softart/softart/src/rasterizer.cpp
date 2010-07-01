#include "../include/rasterizer.h"

#include "../include/shaderregs_op.h"
#include "../include/framebuffer.h"
#include "../include/renderer_impl.h"
#include "../include/cpuinfo.h"
#include "../include/clipper.h"
#include "../include/vertex_cache.h"
#include "../include/thread_pool.h"

#include "eflib/include/slog.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/bind.hpp>
BEGIN_NS_SOFTART()


using namespace std;
using namespace efl;
using namespace boost;

const int TILE_SIZE = 64;
const int GEOMETRY_SETUP_PACKAGE_SIZE = 1;
const int DISPATCH_PRIMITIVE_PACKAGE_SIZE = 1;
const int RASTERIZE_PRIMITIVE_PACKAGE_SIZE = 1;
const int COMPACT_CLIPPED_VERTS_PACKAGE_SIZE = 1;

struct scanline_info
{
	size_t scanline_width;

	vs_output base_vert;
	size_t base_x;

	scanline_info()
	{}

	scanline_info(const scanline_info& rhs)
		:base_vert(rhs.base_vert), scanline_width(scanline_width),
		base_x(rhs.base_x)
	{}

	scanline_info& operator = (const scanline_info& rhs){
		base_vert = rhs.base_vert;
		base_x = rhs.base_x;
	}
};

bool cull_mode_none(float /*area*/)
{
	return false;
}

bool cull_mode_ccw(float area)
{
	return area <= 0;
}

bool cull_mode_cw(float area)
{
	return area >= 0;
}

void fill_wireframe_clipping(uint32_t& num_clipped_verts, vs_output* clipped_verts, uint32_t* clipped_indices, uint32_t base_vertex, const h_clipper& clipper, const viewport& vp, const vs_output** pv, float area)
{
	num_clipped_verts = 0;
	vs_output tmp_verts[6];
	uint32_t num_out_clipped_verts;

	const bool front_face = area > 0;

	clipper->clip(tmp_verts, num_out_clipped_verts, vp, *pv[0], *pv[1]);
	for (uint32_t j = 0; j < num_out_clipped_verts; ++ j){
		clipped_indices[num_clipped_verts + j] = base_vertex + num_clipped_verts + j;
		clipped_verts[num_clipped_verts + j] = tmp_verts[j];
		clipped_verts[num_clipped_verts + j].front_face = front_face;
	}
	num_clipped_verts += num_out_clipped_verts;

	clipper->clip(tmp_verts, num_out_clipped_verts, vp, *pv[1], *pv[2]);
	for (uint32_t j = 0; j < num_out_clipped_verts; ++ j){
		clipped_indices[num_clipped_verts + j] = base_vertex + num_clipped_verts + j;
		clipped_verts[num_clipped_verts + j] = tmp_verts[j];
		clipped_verts[num_clipped_verts + j].front_face = front_face;
	}
	num_clipped_verts += num_out_clipped_verts;
						
	clipper->clip(tmp_verts, num_out_clipped_verts, vp, *pv[2], *pv[0]);
	for (uint32_t j = 0; j < num_out_clipped_verts; ++ j){
		clipped_indices[num_clipped_verts + j] = base_vertex + num_clipped_verts + j;
		clipped_verts[num_clipped_verts + j] = tmp_verts[j];
		clipped_verts[num_clipped_verts + j].front_face = front_face;
	}
	num_clipped_verts += num_out_clipped_verts;
}

void fill_solid_clipping(uint32_t& num_clipped_verts, vs_output* clipped_verts, uint32_t* clipped_indices, uint32_t base_vertex, const h_clipper& clipper, const viewport& vp, const vs_output** pv, float area)
{
	vs_output tmp_verts[6];
	uint32_t num_out_clipped_verts;
	clipper->clip(tmp_verts, num_out_clipped_verts, vp, *pv[0], *pv[1], *pv[2]);
	custom_assert(num_out_clipped_verts <= 6, "");

	num_clipped_verts = (0 == num_out_clipped_verts) ? 0 : (num_out_clipped_verts - 2) * 3;

	const bool front_face = area > 0;

	for (uint32_t i = 0; i < num_out_clipped_verts; ++ i){
		clipped_verts[i] = tmp_verts[i];
		clipped_verts[i].front_face = front_face;
	}

	for(int i_tri = 1; i_tri < static_cast<int>(num_out_clipped_verts) - 1; ++ i_tri){
		clipped_indices[(i_tri - 1) * 3 + 0] = base_vertex + 0;
		if (front_face){
			clipped_indices[(i_tri - 1) * 3 + 1] = base_vertex + i_tri + 0;
			clipped_indices[(i_tri - 1) * 3 + 2] = base_vertex + i_tri + 1;
		}
		else{
			clipped_indices[(i_tri - 1) * 3 + 1] = base_vertex + i_tri + 1;
			clipped_indices[(i_tri - 1) * 3 + 2] = base_vertex + i_tri + 0;
		}
	}
}

void fill_wireframe_triangle_rasterize_func(uint32_t& prim_size, boost::function<void (rasterizer*, const std::vector<uint32_t>&, const std::vector<vs_output>&, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)>& rasterize_func)
{
	prim_size = 2;
	rasterize_func = boost::mem_fn(&rasterizer::rasterize_line_func);
}

void fill_solid_triangle_rasterize_func(uint32_t& prim_size, boost::function<void (rasterizer*, const std::vector<uint32_t>&, const std::vector<vs_output>&, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)>& rasterize_func)
{
	prim_size = 3;
	rasterize_func = boost::mem_fn(&rasterizer::rasterize_triangle_func);
}

rasterizer_state::rasterizer_state(const rasterizer_desc& desc)
	: desc_(desc)
{
	switch (desc.cm)
	{
	case cull_none:
		cm_func_ = cull_mode_none;
		break;

	case cull_front:
		cm_func_ = desc.front_ccw ? cull_mode_ccw : cull_mode_cw;
		break;

	case cull_back:
		cm_func_ = desc.front_ccw ? cull_mode_cw : cull_mode_ccw;
		break;

	default:
		custom_assert(false, "");
		break;
	}
	switch (desc.fm)
	{
	case fill_wireframe:
		clipping_func_ = fill_wireframe_clipping;
		triangle_rast_func_ = fill_wireframe_triangle_rasterize_func;
		break;

	case fill_solid:
		clipping_func_ = fill_solid_clipping;
		triangle_rast_func_ = fill_solid_triangle_rasterize_func;
		break;

	default:
		custom_assert(false, "");
		break;
	}
}

const rasterizer_desc& rasterizer_state::get_desc() const
{
	return desc_;
}

bool rasterizer_state::cull(float area) const
{
	return cm_func_(area);
}

void rasterizer_state::clipping(uint32_t& num_clipped_verts, vs_output* clipped_verts, uint32_t* clipped_indices, uint32_t base_vertex, const h_clipper& clipper, const viewport& vp, const vs_output** pv, float area) const
{
	clipping_func_(num_clipped_verts, clipped_verts, clipped_indices, base_vertex, clipper, vp, pv, area);
}

void rasterizer_state::triangle_rast_func(uint32_t& prim_size, boost::function<void (rasterizer*, const std::vector<uint32_t>&, const std::vector<vs_output>&, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)>& rasterize_func) const
{
	triangle_rast_func_(prim_size, rasterize_func);
}

//inherited
void rasterizer::initialize(renderer_impl* pparent)
{
	pparent_ = pparent;
	hfb_ = pparent->get_framebuffer();
}

/*************************************************
 *   线段的光栅化步骤：
 *			1 寻找主方向，获得主方向距离分量并求得主方向上的差分
 *			2 计算ddx与ddy（用于mip的选择）
 *			3 利用主方向及主方向差分计算像素位置及vs_output
 *			4 执行pixel shader
 *			5 将像素渲染到framebuffer中
 *
 *   Note: 
 *			1 参数的postion将位于窗口坐标系下
 *			2 wpos的x y z分量已经除以了clip w
 *			3 positon.w为1.0f / clip w
 **************************************************/
void rasterizer::rasterize_line(uint32_t /*prim_id*/, const vs_output& v0, const vs_output& v1, const viewport& vp, const h_pixel_shader& pps)
{
	vs_output diff = project(v1) - project(v0);
	const efl::vec4& dir = diff.position;
	float diff_dir = abs(dir.x) > abs(dir.y) ? dir.x : dir.y;

	h_blend_shader hbs = pparent_->get_blend_shader();

	//构造差分
	vs_output ddx = diff * (diff.position.x / (diff.position.xy().length_sqr()));
	vs_output ddy = diff * (diff.position.y / (diff.position.xy().length_sqr()));

	int vpleft = fast_floori(max(0.0f, vp.x));
	int vptop = fast_floori(max(0.0f, vp.y));
	int vpright = fast_floori(min(vp.x+vp.w, (float)(hfb_->get_width())));
	int vpbottom = fast_floori(min(vp.y+vp.h, (float)(hfb_->get_height())));

	ps_output px_out;

	//分为x major和y major使用DDA绘制线
	if( abs(dir.x) > abs(dir.y))
	{

		//调换起终点，使方向递增
		const vs_output *start, *end;
		if(dir.x < 0){
			start = &v1;
			end = &v0;
			diff_dir = -diff_dir;
		} else {
			start = &v0;
			end = &v1;
		}

		triangle_info info;
		info.set(start->position, ddx, ddy);
		pps->ptriangleinfo_ = &info;

		float fsx = fast_floor(start->position.x + 0.5f);

		int sx = fast_floori(fsx);
		int ex = fast_floori(end->position.x - 0.5f);

		//截取到屏幕内
		sx = efl::clamp<int>(sx, vpleft, int(vpright - 1));
		ex = efl::clamp<int>(ex, vpleft, int(vpright));

		//设置起点的vs_output
		vs_output px_start(project(*start));
		vs_output px_end(project(*end));
		float step = sx + 0.5f - start->position.x;
		vs_output px_in = lerp(px_start, px_end, step / diff_dir);

		//x-major 的线绘制
		vs_output unprojed;
		for(int iPixel = sx; iPixel < ex; ++iPixel)
		{
			//忽略不在vp范围内的像素
			if(px_in.position.y >= vpbottom){
				if(dir.y > 0) break;
				continue;
			}
			if(px_in.position.y < 0){
				if(dir.y < 0) break;
				continue;
			}

			//进行像素渲染
			unproject(unprojed, px_in);
			if(pps->execute(unprojed, px_out)){
				hfb_->render_pixel(hbs, iPixel, fast_floori(px_in.position.y), px_out, &px_out.depth);
			}

			//差分递增
			++ step;
			px_in = lerp(px_start, px_end, step / diff_dir);
		}
	}
	else //y major
	{
		//调换序列依据方向
		const vs_output *start, *end;
		if(dir.y < 0){
			start = &v1;
			end = &v0;
			diff_dir = -diff_dir;
		} else {
			start = &v0;
			end = &v1;
		}

		triangle_info info;
		info.set(start->position, ddx, ddy);
		pps->ptriangleinfo_ = &info;

		float fsy = fast_floor(start->position.y + 0.5f);

		int sy = fast_floori(fsy);
		int ey = fast_floori(end->position.y - 0.5f);

		//截取到屏幕内
		sy = efl::clamp<int>(sy, vptop, int(vpbottom - 1));
		ey = efl::clamp<int>(ey, vptop, int(vpbottom));

		//设置起点的vs_output
		vs_output px_start(project(*start));
		vs_output px_end(project(*end));
		float step = sy + 0.5f - start->position.y;
		vs_output px_in = lerp(px_start, px_end, step / diff_dir);

		//x-major 的线绘制
		vs_output unprojed;
		for(int iPixel = sy; iPixel < ey; ++iPixel)
		{
			//忽略不在vp范围内的像素
			if(px_in.position.x >= vpright){
				if(dir.x > 0) break;
				continue;
			}
			if(px_in.position.x < 0){
				if(dir.x < 0) break;
				continue;
			}

			//进行像素渲染
			unproject(unprojed, px_in);
			if(pps->execute(unprojed, px_out)){
				hfb_->render_pixel(hbs, fast_floori(px_in.position.x), iPixel, px_out, &px_out.depth);
			}

			//差分递增
			++ step;
			px_in = lerp(px_start, px_end, step / diff_dir);
		}
	}
}

/*************************************************
*   三角形的光栅化步骤：
*			1 光栅化生成扫描线及扫描线差分信息
*			2 rasterizer_scanline_impl处理扫描线
*			3 生成逐个像素的vs_output
*			4 执行pixel shader
*			5 将像素渲染到framebuffer中
*
*   Note: 
*			1 参数的postion将位于窗口坐标系下
*			2 wpos的x y z分量已经除以了clip w
*			3 positon.w为1.0f / clip w
**************************************************/
void rasterizer::rasterize_triangle(uint32_t prim_id, const vs_output& v0, const vs_output& v1, const vs_output& v2, const viewport& vp, const h_pixel_shader& pps)
{

	//{
	//	boost::mutex::scoped_lock lock(logger_mutex_);
	//
	//	typedef slog<text_log_serializer> slog_type;
	//	log_serializer_indent_scope<log_system<slog_type>::slog_type> scope(&log_system<slog_type>::instance());

	//	//记录三角形的屏幕坐标系顶点。
	//	log_system<slog_type>::instance().write(_EFLIB_T("wv0"),
	//		to_tstring(str(format("( %1%, %2%, %3%)") % v0.wpos.x % v0.wpos.y % v0.wpos.z)), LOGLEVEL_MESSAGE
	//		);
	//	log_system<slog_type>::instance().write(_EFLIB_T("wv1"), 
	//		to_tstring(str(format("( %1%, %2%, %3%)") % v1.wpos.x % v1.wpos.y % v1.wpos.z)), LOGLEVEL_MESSAGE
	//		);
	//	log_system<slog_type>::instance().write(_EFLIB_T("wv2"), 
	//		to_tstring(str(format("( %1%, %2%, %3%)") % v2.wpos.x % v2.wpos.y % v2.wpos.z)), LOGLEVEL_MESSAGE
	//		);
	//}

	/**********************************************************
	*        将顶点按照y大小排序，求出三角形面积与边
	**********************************************************/
	const vs_output* pvert[3] = {&v0, &v1, &v2};
	const vec3* edge_factors = &edge_factors_[prim_id * 3];
	const int min_corner[3] = {
		(edge_factors[0].y > 0) * 2 + (edge_factors[0].x <= 0),
		(edge_factors[1].y > 0) * 2 + (edge_factors[1].x <= 0),
		(edge_factors[2].y > 0) * 2 + (edge_factors[2].x <= 0)
	};

	//升序排列
	if(pvert[0]->position.y > pvert[1]->position.y){
		swap(pvert[1], pvert[0]);
	}
	if(pvert[1]->position.y > pvert[2]->position.y){
		swap(pvert[2], pvert[1]);
		if(pvert[0]->position.y > pvert[1]->position.y) 
			swap(pvert[1], pvert[0]);
	}

	vs_output projed_vert0 = project(*(pvert[0]));

	//初始化边及边上属性的差
	vs_output e01 = project(*(pvert[1])) - projed_vert0;
	//float watch_x = e01.attributes[2].x;
	
	vs_output e02 = project(*(pvert[2])) - projed_vert0;
	vs_output e12;



	//初始化边上的各个分量差值。（只要算两条边就可以了。）
	e12.position = pvert[2]->position - pvert[1]->position;

	//计算面积
	float area = cross_prod2(e02.position.xy(), e01.position.xy());
	float inv_area = 1.0f / area;
	if(equal<float>(area, 0.0f)) return;

	/**********************************************************
	*  求解各个属性的差分式
	*********************************************************/
	vs_output ddx((e02 * e01.position.y - e02.position.y * e01)*inv_area);
	vs_output ddy((e01 * e02.position.x - e01.position.x * e02)*inv_area);

	triangle_info info;
	info.set(pvert[0]->position, ddx, ddy);
	pps->ptriangleinfo_ = &info;

	/*************************************
	*   设置基本的scanline属性。
	*   这些属性将在多个扫描行中保持相同
	************************************/
	scanline_info scanline;

	/*************************************************
	*   开始绘制多边形。经典的上-下分割绘制算法
	*   对扫描线光栅化保证是自左向右的。
	*   所以不需要考虑major edge是在左或者右边。
	*************************************************/

	enum TRI_VS_TILE {
		TVT_ALL,
		TVT_PART,
		TVT_NONE
	};

	const h_blend_shader& hbs = pparent_->get_blend_shader();
	const size_t num_samples = hfb_->get_num_samples();

	std::vector<efl::rect<uint8_t> > test_regions[2];
	test_regions[0].push_back(efl::rect<uint8_t>(0, 0, static_cast<uint8_t>(vp.w), static_cast<uint8_t>(vp.h)));
	int src_stage = 0;
	int dst_stage = !src_stage;

	while (!test_regions[src_stage].empty()){
		test_regions[dst_stage].clear();

		for (size_t ivp = 0; ivp < test_regions[src_stage].size(); ++ ivp){
			const efl::rect<uint8_t>& cur_region = test_regions[src_stage][ivp];

			const int vpleft = fast_floori(max(0.0f, vp.x + cur_region.x));
			const int vptop = fast_floori(max(0.0f, vp.y + cur_region.y));
			const int vpright = fast_floori(min(vp.x + cur_region.x + cur_region.w, static_cast<float>(hfb_->get_width())));
			const int vpbottom = fast_floori(min(vp.y + cur_region.y + cur_region.h, static_cast<float>(hfb_->get_height())));

			const int2 corners[] = {
				int2(vpleft, vptop),
				int2(vpright, vptop),
				int2(vpleft, vpbottom),
				int2(vpright, vpbottom)
			};

			TRI_VS_TILE intersect = TVT_ALL;
			
			// Trival rejection
			for (int e = 0; e < 3; ++ e){
				int min_c = min_corner[e];
				if (min_c != (ivp & 3)){
					if (corners[min_c].x * edge_factors[e].x
							- corners[min_c].y * edge_factors[e].y
							- edge_factors[e].z > 0){
						intersect = TVT_NONE;
						break;
					}
				}
			}
			// Trival acception
			if (intersect != TVT_NONE){
				for (int e = 0; e < 3; ++ e){
					int min_c = 3 - min_corner[e];
					if (corners[min_c].x * edge_factors[e].x
							- corners[min_c].y * edge_factors[e].y
							- edge_factors[e].z > 0){
						intersect = TVT_PART;
						break;
					}
				}
			}
			// For one pixel region
			if ((TVT_PART == intersect) && (cur_region.w <= 1) && (cur_region.h <= 1)){
				intersect = TVT_ALL;
			}

			switch (intersect)
			{
			case TVT_NONE:
				break;

			case TVT_ALL: 
				{
					const float offsetx = vpleft + 0.5f - pvert[0]->position.x;
					const float offsety = vptop + 0.5f - pvert[0]->position.y;

					//设置基准扫描线的属性
					scanline.base_vert = projed_vert0;
					integral(scanline.base_vert, offsety, ddy);
					integral(scanline.base_vert, offsetx, ddx);

					//当前的基准扫描线，起点在(base_vert.x, scanline.y)处。
					//在传递到rasterize_scanline之前需要将基础点调整到扫描线的最左端。
					scanline.base_x = vpleft;
					scanline.scanline_width = vpright - vpleft;

					for(int iy = vptop; iy < vpbottom; ++iy)
					{	
						//光栅化
						vs_output px_in(scanline.base_vert);
						ps_output px_out;
						vs_output unprojed;
						for(size_t i_pixel = 0; i_pixel < scanline.scanline_width; ++i_pixel)
						{
							const size_t ix = scanline.base_x + i_pixel;

							unproject(unprojed, px_in);
							if(pps->execute(unprojed, px_out)){
								float samples_depth[MAX_NUM_MULTI_SAMPLES];
								for (int i_sample = 0; i_sample < num_samples; ++ i_sample){
									const vec2& sp = samples_pattern_[i_sample];
									const float fx = ix + sp.x;
									const float fy = iy + sp.y;
									bool inside = true;
									for (int e = 0; e < 3; ++ e){
										if (fx * edge_factors[e].x
												- fy * edge_factors[e].y
												- edge_factors[e].z > 0){
											inside = false;
											break;
										}
									}
									if (inside){
										const float ddxz = (sp.x - 0.5f) * ddx.position.z;
										const float ddyz = (sp.y - 0.5f) * ddy.position.z;
										samples_depth[i_sample] = px_in.position.z + ddxz + ddyz;
									}
									else{
										samples_depth[i_sample] = 1;
									}
								}

								hfb_->render_pixel(hbs, ix, iy, px_out, samples_depth);
							}

							integral(px_in, 1.0f, ddx);
						}

						//差分递增
						integral(scanline.base_vert, 1.0f, ddy);
					}
				}
				break;

			default:
				{
					const uint8_t new_w = std::max<uint8_t>(1, cur_region.w / 2);
					const uint8_t new_h = std::max<uint8_t>(1, cur_region.h / 2);

					efl::rect<uint8_t> tmp_region;

					tmp_region.x = cur_region.x;
					tmp_region.y = cur_region.y;
					tmp_region.w = new_w;
					tmp_region.h = new_h;
					test_regions[dst_stage].push_back(tmp_region);

					tmp_region.x = cur_region.x + new_w;
					tmp_region.y = cur_region.y;
					tmp_region.w = new_w;
					tmp_region.h = new_h;
					test_regions[dst_stage].push_back(tmp_region);

					tmp_region.x = cur_region.x;
					tmp_region.y = cur_region.y + new_h;
					tmp_region.w = new_w;
					tmp_region.h = new_h;
					test_regions[dst_stage].push_back(tmp_region);

					tmp_region.x = cur_region.x + new_w;
					tmp_region.y = cur_region.y + new_h;
					tmp_region.w = new_w;
					tmp_region.h = new_h;
					test_regions[dst_stage].push_back(tmp_region);
				}
				break;
			}
		}

		src_stage = (src_stage + 1) & 1;
		dst_stage = !src_stage;
	}
}

rasterizer::rasterizer()
{
	state_.reset(new rasterizer_state(rasterizer_desc()));
}
rasterizer::~rasterizer()
{
}

void rasterizer::set_state(const h_rasterizer_state& state)
{
	state_ = state;
}

const h_rasterizer_state& rasterizer::get_state() const
{
	return state_;
}

void rasterizer::geometry_setup_func(std::vector<uint32_t>& num_clipped_verts, std::vector<vs_output>& clipped_verts, std::vector<uint32_t>& cliped_indices,
		int32_t prim_count, primitive_topology primtopo, atomic<int32_t>& working_package, int32_t package_size){

	const int32_t num_packages = (prim_count + package_size - 1) / package_size;

	const h_vertex_cache& dvc = pparent_->get_vertex_cache();
	const h_clipper& clipper = pparent_->get_clipper();
	const viewport& vp = pparent_->get_viewport();

	uint32_t prim_size = 0;
	switch(primtopo)
	{
	case primitive_line_list:
	case primitive_line_strip:
		prim_size = 2;
		break;
	case primitive_triangle_list:
	case primitive_triangle_strip:
		prim_size = 3;
		break;
	default:
		custom_assert(false, "枚举值无效：无效的Primitive Topology");
		return;
	}

	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages){
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(prim_count, start + package_size);
		for (int32_t i = start; i < end; ++ i){
			if (3 == prim_size){
				const vs_output* pv[3];
				vec2 pv_2d[3];
				for (size_t j = 0; j < 3; ++ j){
					const vs_output& v = dvc->fetch(i * 3 + j);
					pv[j] = &v;
					const float abs_w = abs(v.position.w);
					const float x = v.position.x / abs_w;
					const float y = v.position.y / abs_w;
					pv_2d[j] = vec2(x, y);
				}

				const float area = cross_prod2(pv_2d[2] - pv_2d[0], pv_2d[1] - pv_2d[0]);
				if (!state_->cull(area)){
					state_->clipping(num_clipped_verts[i], &clipped_verts[i * 6], &cliped_indices[i * 12], i * 6, clipper, vp, pv, area);
				}
				else{
					num_clipped_verts[i] = 0;
				}
			}
			else if (2 == prim_size){
				vs_output pv[2];
				for (size_t j = 0; j < prim_size; ++ j){
					pv[j] = dvc->fetch(i * prim_size + j);
				}

				vs_output tmp_verts[6];
				uint32_t num_out_clipped_verts;
				clipper->clip(tmp_verts, num_out_clipped_verts, vp, pv[0], pv[1]);
				num_clipped_verts[i] = num_out_clipped_verts;
				for (uint32_t j = 0; j < num_out_clipped_verts; ++ j){
					clipped_verts[i * 6 + j] = tmp_verts[j];
					cliped_indices[i * 12 + j] = static_cast<uint32_t>(i * 6 + j);
				}
			}
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::dispatch_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles,
		const std::vector<uint32_t>& clipped_indices, const std::vector<vs_output>& clipped_verts_full, int32_t prim_count, uint32_t stride, atomic<int32_t>& working_package, int32_t package_size){

	const viewport& vp = pparent_->get_viewport();
	int num_tiles_x = static_cast<size_t>(vp.w + TILE_SIZE - 1) / TILE_SIZE;
	int num_tiles_y = static_cast<size_t>(vp.h + TILE_SIZE - 1) / TILE_SIZE;

	const int32_t num_packages = (prim_count + package_size - 1) / package_size;
	
	float x_min;
	float x_max;
	float y_min;
	float y_max;
	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages)
	{
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(prim_count, start + package_size);
		for (int32_t i = start; i < end; ++ i)
		{
			vec2 pv[3];
			for (size_t j = 0; j < stride; ++ j){
				pv[j] = clipped_verts_full[clipped_indices[i * stride + j]].position.xy();
			}

			if (3 == stride){
				// x * (y1 - y0) - y * (x1 - x0) - (y1 * x0 - x1 * y0)
				vec3* edge_factors = &edge_factors_[i * 3];
				for (int e = 0; e < 3; ++ e){
					const int se = e;
					const int ee = (e + 1) % 3;
					edge_factors[e].x = pv[ee].y - pv[se].y;
					edge_factors[e].y = pv[ee].x - pv[se].x;
					edge_factors[e].z = pv[ee].y * pv[se].x - pv[ee].x * pv[se].y;
				}
			}

			x_min = pv[0].x;
			x_max = pv[0].x;
			y_min = pv[0].y;
			y_max = pv[0].y;
			for (size_t j = 1; j < stride; ++ j){
				x_min = min(x_min, pv[j].x);
				x_max = max(x_max, pv[j].x);
				y_min = min(y_min, pv[j].y);
				y_max = max(y_max, pv[j].y);
			}

			const int sx = std::min(fast_floori(std::max(0.0f, x_min) / TILE_SIZE), num_tiles_x);
			const int sy = std::min(fast_floori(std::max(0.0f, y_min) / TILE_SIZE), num_tiles_y);
			const int ex = std::min(fast_ceili(std::max(0.0f, x_max) / TILE_SIZE) + 1, num_tiles_x);
			const int ey = std::min(fast_ceili(std::max(0.0f, y_max) / TILE_SIZE) + 1, num_tiles_y);
			for (int y = sy; y < ey; ++ y){
				for (int x = sx; x < ex; ++ x){
					tiles[y * num_tiles_x + x].enqueue(i);
				}
			}
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::rasterize_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int num_tiles_x,
		const std::vector<uint32_t>& clipped_indices, const std::vector<vs_output>& clipped_verts_full, const h_pixel_shader& pps, atomic<int32_t>& working_package, int32_t package_size)
{
	const int32_t num_tiles = static_cast<int32_t>(tiles.size());
	const int32_t num_packages = (num_tiles + package_size - 1) / package_size;

	const viewport& vp = pparent_->get_viewport();

	viewport tile_vp;
	tile_vp.w = TILE_SIZE;
	tile_vp.h = TILE_SIZE;
	tile_vp.minz = vp.minz;
	tile_vp.maxz = vp.maxz;

	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages){
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(num_tiles, start + package_size);
		for (int32_t i = start; i < end; ++ i){
			lockfree_queue<uint32_t>& prims = tiles[i];

			std::vector<uint32_t> sorted_prims;
			prims.dequeue_all(std::back_insert_iterator<std::vector<uint32_t> >(sorted_prims));
			std::sort(sorted_prims.begin(), sorted_prims.end());

			int y = i / num_tiles_x;
			int x = i - y * num_tiles_x;
			tile_vp.x = static_cast<float>(x * TILE_SIZE);
			tile_vp.y = static_cast<float>(y * TILE_SIZE);

			rasterize_func_(this, clipped_indices, clipped_verts_full, sorted_prims, tile_vp, pps);
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::rasterize_line_func(const std::vector<uint32_t>& clipped_indices, const std::vector<vs_output>& clipped_verts_full, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps){
	for (std::vector<uint32_t>::const_iterator iter = sorted_prims.begin(); iter != sorted_prims.end(); ++ iter){
		uint32_t iprim = *iter;
		this->rasterize_line(iprim, clipped_verts_full[clipped_indices[iprim * 2 + 0]], clipped_verts_full[clipped_indices[iprim * 2 + 1]], tile_vp, pps);
	}
}

void rasterizer::rasterize_triangle_func(const std::vector<uint32_t>& clipped_indices, const std::vector<vs_output>& clipped_verts_full, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps){
	for (std::vector<uint32_t>::const_iterator iter = sorted_prims.begin(); iter != sorted_prims.end(); ++ iter){
		uint32_t iprim = *iter;
		this->rasterize_triangle(iprim, clipped_verts_full[clipped_indices[iprim * 3 + 0]], clipped_verts_full[clipped_indices[iprim * 3 + 1]], clipped_verts_full[clipped_indices[iprim * 3 + 2]], tile_vp, pps);
	}
}

void rasterizer::compact_clipped_verts_func(std::vector<uint32_t>& clipped_indicess, const std::vector<uint32_t>& clipped_indices_full,
		const std::vector<uint32_t>& addresses, const std::vector<uint32_t>& num_clipped_verts, int32_t prim_count,
		atomic<int32_t>& working_package, int32_t package_size){
	const int32_t num_packages = (prim_count + package_size - 1) / package_size;
	
	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages)
	{
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(prim_count, start + package_size);
		for (int32_t i = start; i < end; ++ i)
		{
			const uint32_t addr = addresses[i];
			for (uint32_t j = 0; j < num_clipped_verts[i]; ++ j){
				clipped_indicess[addr + j] = clipped_indices_full[i * 12 + j];
			}
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::draw(size_t prim_count){

	custom_assert(pparent_, "Renderer 指针为空！可能该对象没有经过正确的初始化！");
	if(!pparent_) return;

	const size_t num_samples = hfb_->get_num_samples();
	switch (num_samples){
	case 1:
		samples_pattern_[0] = vec2(0.5f, 0.5f);
		break;

	case 2:
		samples_pattern_[0] = vec2(0.25f, 0.25f);
		samples_pattern_[1] = vec2(0.75f, 0.75f);
		break;

	case 4:
		samples_pattern_[0] = vec2(0.375f, 0.125f);
		samples_pattern_[1] = vec2(0.875f, 0.375f);
		samples_pattern_[2] = vec2(0.125f, 0.625f);
		samples_pattern_[3] = vec2(0.625f, 0.875f);
		break;

	default:
		break;
	}

	primitive_topology primtopo = pparent_->get_primitive_topology();

	uint32_t prim_size = 0;
	switch(primtopo)
	{
	case primitive_line_list:
	case primitive_line_strip:
		prim_size = 2;
		rasterize_func_ = boost::mem_fn(&rasterizer::rasterize_line_func);
		break;
	case primitive_triangle_list:
	case primitive_triangle_strip:
		state_->triangle_rast_func(prim_size, rasterize_func_);
		break;
	default:
		custom_assert(false, "枚举值无效：无效的Primitive Topology");
		return;
	}

	const viewport& vp = pparent_->get_viewport();
	int num_tiles_x = static_cast<size_t>(vp.w + TILE_SIZE - 1) / TILE_SIZE;
	int num_tiles_y = static_cast<size_t>(vp.h + TILE_SIZE - 1) / TILE_SIZE;
	std::vector<lockfree_queue<uint32_t> > tiles(num_tiles_x * num_tiles_y);

	atomic<int32_t> working_package(0);
	size_t num_threads = num_available_threads();

	// Culling, Clipping, Geometry setup
	std::vector<uint32_t> num_clipped_verts(prim_count);
	std::vector<vs_output> clipped_verts_full(prim_count * 6);
	std::vector<uint32_t> clipped_indices_full(prim_count * 12);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		global_thread_pool().schedule(boost::bind(&rasterizer::geometry_setup_func, this, boost::ref(num_clipped_verts),
			boost::ref(clipped_verts_full), boost::ref(clipped_indices_full), static_cast<int32_t>(prim_count), primtopo,
			working_package, GEOMETRY_SETUP_PACKAGE_SIZE));
	}
	geometry_setup_func(boost::ref(num_clipped_verts), boost::ref(clipped_verts_full), boost::ref(clipped_indices_full),
		static_cast<int32_t>(prim_count), primtopo, working_package, GEOMETRY_SETUP_PACKAGE_SIZE);
	global_thread_pool().wait();

	std::vector<uint32_t> addresses(prim_count);
	addresses[0] = 0;
	for (size_t i = 1; i < prim_count; ++ i){
		addresses[i] = addresses[i - 1] + num_clipped_verts[i - 1];
	}

	std::vector<uint32_t> clipped_indices(addresses.back() + num_clipped_verts.back());
	working_package = 0;
	for (size_t i = 0; i < num_threads - 1; ++ i){
		global_thread_pool().schedule(boost::bind(&rasterizer::compact_clipped_verts_func, this, boost::ref(clipped_indices),
			boost::ref(clipped_indices_full), boost::ref(addresses),
			boost::ref(num_clipped_verts), prim_count, working_package, COMPACT_CLIPPED_VERTS_PACKAGE_SIZE));
	}
	compact_clipped_verts_func(boost::ref(clipped_indices), boost::ref(clipped_indices_full), boost::ref(addresses),
			boost::ref(num_clipped_verts), static_cast<int32_t>(prim_count), working_package, COMPACT_CLIPPED_VERTS_PACKAGE_SIZE);
	global_thread_pool().wait();

	working_package = 0;
	edge_factors_.resize(clipped_indices.size() / prim_size * 3);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		global_thread_pool().schedule(boost::bind(&rasterizer::dispatch_primitive_func, this, boost::ref(tiles),
			boost::ref(clipped_indices), boost::ref(clipped_verts_full), static_cast<int32_t>(clipped_indices.size() / prim_size),
			prim_size, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE));
	}
	dispatch_primitive_func(boost::ref(tiles),
		boost::ref(clipped_indices), boost::ref(clipped_verts_full), static_cast<int32_t>(clipped_indices.size() / prim_size),
		prim_size, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE);
	global_thread_pool().wait();

	working_package = 0;
	h_pixel_shader hps = pparent_->get_pixel_shader();
	std::vector<h_pixel_shader> ppps(num_threads - 1);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		// create pixel_shader clone per thread from hps
		ppps[i] = hps->create_clone();
		global_thread_pool().schedule(boost::bind(&rasterizer::rasterize_primitive_func, this, boost::ref(tiles),
			num_tiles_x, boost::ref(clipped_indices), boost::ref(clipped_verts_full), ppps[i], boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE));
	}
	rasterize_primitive_func(boost::ref(tiles), num_tiles_x, boost::ref(clipped_indices), boost::ref(clipped_verts_full), hps, boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE);
	global_thread_pool().wait();
	// destroy all pixel_shader clone
	for (size_t i = 0; i < num_threads - 1; ++ i){
		hps->destroy_clone(ppps[i]);
	}
}

END_NS_SOFTART()
