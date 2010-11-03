#include "../include/rasterizer.h"

#include "../include/shaderregs_op.h"
#include "../include/framebuffer.h"
#include "../include/renderer_impl.h"
#include "../include/cpuinfo.h"
#include "../include/clipper.h"
#include "../include/vertex_cache.h"
#include "../include/thread_pool.h"

#include <eflib/include/diagnostics/log.h>
#include <eflib/include/metaprog/util.h>

#include <algorithm>
#include <boost/format.hpp>
#include <boost/bind.hpp>
BEGIN_NS_SOFTART()

//#define USE_TRADITIONAL_RASTERIZER

using namespace std;
using namespace eflib;
using namespace boost;

const int TILE_SIZE = 64;
const int GEOMETRY_SETUP_PACKAGE_SIZE = 8;
const int DISPATCH_PRIMITIVE_PACKAGE_SIZE = 4;
const int RASTERIZE_PRIMITIVE_PACKAGE_SIZE = 1;
const int COMPACT_CLIPPED_VERTS_PACKAGE_SIZE = 8;

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
	uint32_t num_out_clipped_verts;

	const bool front_face = area > 0;

	clipper->clip(&clipped_verts[num_clipped_verts], num_out_clipped_verts, vp, *pv[0], *pv[1]);
	for (uint32_t j = 0; j < num_out_clipped_verts; ++ j){
		clipped_indices[num_clipped_verts + j] = base_vertex + num_clipped_verts + j;
		clipped_verts[num_clipped_verts + j].front_face = front_face;
	}
	num_clipped_verts += num_out_clipped_verts;

	clipper->clip(&clipped_verts[num_clipped_verts], num_out_clipped_verts, vp, *pv[1], *pv[2]);
	for (uint32_t j = 0; j < num_out_clipped_verts; ++ j){
		clipped_indices[num_clipped_verts + j] = base_vertex + num_clipped_verts + j;
		clipped_verts[num_clipped_verts + j].front_face = front_face;
	}
	num_clipped_verts += num_out_clipped_verts;
						
	clipper->clip(&clipped_verts[num_clipped_verts], num_out_clipped_verts, vp, *pv[2], *pv[0]);
	for (uint32_t j = 0; j < num_out_clipped_verts; ++ j){
		clipped_indices[num_clipped_verts + j] = base_vertex + num_clipped_verts + j;
		clipped_verts[num_clipped_verts + j].front_face = front_face;
	}
	num_clipped_verts += num_out_clipped_verts;
}

void fill_solid_clipping(uint32_t& num_clipped_verts, vs_output* clipped_verts, uint32_t* clipped_indices, uint32_t base_vertex, const h_clipper& clipper, const viewport& vp, const vs_output** pv, float area)
{
	uint32_t num_out_clipped_verts;
	clipper->clip(clipped_verts, num_out_clipped_verts, vp, *pv[0], *pv[1], *pv[2]);
	EFLIB_ASSERT(num_out_clipped_verts <= 6, "");

	num_clipped_verts = (0 == num_out_clipped_verts) ? 0 : (num_out_clipped_verts - 2) * 3;

	const bool front_face = area > 0;

	for (uint32_t i = 0; i < num_out_clipped_verts; ++ i){
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

void fill_wireframe_triangle_rasterize_func(uint32_t& prim_size, boost::function<void (rasterizer*, const uint32_t*, const vs_output*, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)>& rasterize_func)
{
	prim_size = 2;
	rasterize_func = boost::mem_fn(&rasterizer::rasterize_line_func);
}

void fill_solid_triangle_rasterize_func(uint32_t& prim_size, boost::function<void (rasterizer*, const uint32_t*, const vs_output*, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)>& rasterize_func)
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
		EFLIB_ASSERT(false, "");
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
		EFLIB_ASSERT(false, "");
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

void rasterizer_state::triangle_rast_func(uint32_t& prim_size, boost::function<void (rasterizer*, const uint32_t*, const vs_output*, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)>& rasterize_func) const
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
	const eflib::vec4& dir = diff.position;
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
		sx = eflib::clamp<int>(sx, vpleft, int(vpright - 1));
		ex = eflib::clamp<int>(ex, vpleft, int(vpright));

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
				hfb_->render_sample(hbs, iPixel, fast_floori(px_in.position.y), 0, px_out, px_out.depth);
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
		sy = eflib::clamp<int>(sy, vptop, int(vpbottom - 1));
		ey = eflib::clamp<int>(ey, vptop, int(vpbottom));

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
				hfb_->render_sample(hbs, fast_floori(px_in.position.x), iPixel, 0, px_out, px_out.depth);
			}

			//差分递增
			++ step;
			px_in = lerp(px_start, px_end, step / diff_dir);
		}
	}
}

void rasterizer::draw_whole_tile(uint8_t* pixel_begin, uint8_t* pixel_end, uint32_t* pixel_mask, int left, int top, int right, int bottom, uint32_t full_mask){
	for(int iy = top; iy < bottom; ++iy)
	{
		pixel_begin[iy] = static_cast<uint8_t>(min(static_cast<int>(pixel_begin[iy]), left));
		pixel_end[iy] = static_cast<uint8_t>(max(static_cast<int>(pixel_end[iy]), right));
		for(int ix = left; ix < right; ++ix)
		{
			pixel_mask[iy * TILE_SIZE + ix] = full_mask;
		}
	}
}

void rasterizer::draw_pixels(uint8_t* pixel_begin, uint8_t* pixel_end, uint32_t* pixel_mask, int left0, int top0, int left, int top, const eflib::vec3* edge_factors, size_t num_samples){
	float evalue[3];
	for (int e = 0; e < 3; ++ e){
		evalue[e] = edge_factors[e].z - (left * edge_factors[e].x + top * edge_factors[e].y);
	}

	size_t sx = left - left0;
	size_t sy = top - top0;

#ifndef EFLIB_NO_SIMD
	const __m128 mtx = _mm_set_ps(1, 0, 1, 0);
	const __m128 mty = _mm_set_ps(1, 1, 0, 0);

	for (size_t i_sample = 0; i_sample < num_samples; ++ i_sample){
		const vec2& sp = samples_pattern_[i_sample];
		__m128 mspx = _mm_set1_ps(sp.x);
		__m128 mspy = _mm_set1_ps(sp.y);

		__m128 mask_rej = _mm_setzero_ps();
		for (int e = 0; e < 3; ++ e){
			__m128 mstepx = _mm_set_ps1(edge_factors[e].x);
			__m128 mstepy = _mm_set_ps1(edge_factors[e].y);
			__m128 mx = _mm_add_ps(mtx, mspx);
			__m128 my = _mm_add_ps(mty, mspy);
			__m128 msteprej = _mm_add_ps(_mm_mul_ps(mx, mstepx), _mm_mul_ps(my, mstepy));

			__m128 mevalue = _mm_set1_ps(evalue[e]);

			mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
		}

		int sample_inside = ~_mm_movemask_ps(mask_rej);

		for (int t = 0; t < 4; ++ t){
			const size_t x = sx + (t & 1);
			const size_t y = sy + (t >> 1);
			pixel_mask[y * TILE_SIZE + x] = 0;
			if ((sample_inside >> t) & 1){
				pixel_begin[y] = static_cast<uint8_t>(min(static_cast<size_t>(pixel_begin[y]), x));
				pixel_end[y] = static_cast<uint8_t>(max(static_cast<size_t>(pixel_end[y]), x + 1));

				pixel_mask[y * TILE_SIZE + x] |= 1UL << i_sample;
			}
		}
	}
#else
	for(int iy = 0; iy < 2; ++iy)
	{
		//光栅化
		for(size_t ix = 0; ix < 2; ++ix)
		{
			pixel_mask[(iy + sy) * TILE_SIZE + (ix + sx)] = 0;
			for (int i_sample = 0; i_sample < num_samples; ++ i_sample){
				const vec2& sp = samples_pattern_[i_sample];
				const float fx = ix + sp.x;
				const float fy = iy + sp.y;
				bool inside = true;
				for (int e = 0; e < 3; ++ e){
					if (fx * edge_factors[e].x + fy * edge_factors[e].y < evalue[e]){
						inside = false;
						break;
					}
				}
				if (inside){
					const size_t x = ix + sx;
					const size_t y = iy + sy;
					pixel_begin[y] = static_cast<uint8_t>(min(static_cast<size_t>(pixel_begin[y]), x));
					pixel_end[y] = static_cast<uint8_t>(max(static_cast<size_t>(pixel_end[y]), x + 1));

					pixel_mask[y * TILE_SIZE + x] |= 1UL << i_sample;
				}
			}
		}
	}
#endif
}

void rasterizer::subdivide_tile(int left, int top, const eflib::rect<uint32_t>& cur_region, const vec3* edge_factors, const bool* mark_x, const bool* mark_y,
		uint32_t* test_regions, uint32_t& test_region_size, float x_min, float x_max, float y_min, float y_max){
	const uint32_t new_w = std::max<uint32_t>(1, cur_region.w / 2);
	const uint32_t new_h = std::max<uint32_t>(1, cur_region.h / 2);

#ifndef EFLIB_NO_SIMD
	UNREF_PARAM(mark_x);
	UNREF_PARAM(mark_y);

	static const union
	{
		int maski;
		float maskf;
	} MASK = { 0x80000000 };
	static const __m128 SIGN_MASK = _mm_set1_ps(MASK.maskf);

	__m128i mineww = _mm_set1_epi32(new_w);
	__m128i minewh = _mm_set1_epi32(new_h);
	__m128 mneww = _mm_cvtepi32_ps(mineww);
	__m128 mnewh = _mm_cvtepi32_ps(minewh);

	__m128 medgex = _mm_set_ps(0, edge_factors[2].x, edge_factors[1].x, edge_factors[0].x);
	__m128 medgey = _mm_set_ps(0, edge_factors[2].y, edge_factors[1].y, edge_factors[0].y);
	__m128 medgez = _mm_set_ps(0, edge_factors[2].z, edge_factors[1].z, edge_factors[0].z);

	__m128 mstepx3 = _mm_mul_ps(mneww, medgex);
	__m128 mstepy3 = _mm_mul_ps(mnewh, medgey);
	__m128 mrej2acc3 = _mm_add_ps(_mm_or_ps(mstepx3, SIGN_MASK), _mm_or_ps(mstepy3, SIGN_MASK));

	__m128 mleft = _mm_set1_ps(left);
	__m128 mtop = _mm_set1_ps(top);
	__m128 mmarkx = _mm_and_ps(_mm_cmpgt_ps(medgex, _mm_setzero_ps()), mneww);
	__m128 mmarky = _mm_and_ps(_mm_cmpgt_ps(medgey, _mm_setzero_ps()), mnewh);
	__m128 mevalue3 = _mm_sub_ps(medgez, _mm_add_ps(_mm_mul_ps(_mm_add_ps(mleft, mmarkx), medgex), _mm_mul_ps(_mm_add_ps(mtop, mmarky), medgey)));

	__m128 mask_rej = _mm_setzero_ps();
	__m128 mask_acc = _mm_setzero_ps();
	// Trival rejection & acception
	{
		__m128 mstepx = _mm_shuffle_ps(mstepx3, mstepx3, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 mstepy = _mm_shuffle_ps(mstepy3, mstepy3, _MM_SHUFFLE(0, 0, 0, 0));

		__m128 mrej2acc = _mm_shuffle_ps(mrej2acc3, mrej2acc3, _MM_SHUFFLE(0, 0, 0, 0));

		__m128 msteprej = _mm_add_ps(_mm_unpacklo_ps(_mm_setzero_ps(), mstepx), _mm_movelh_ps(_mm_setzero_ps(), mstepy));
		__m128 mstepacc = _mm_add_ps(msteprej, mrej2acc);

		__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(0, 0, 0, 0));

		mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
		mask_acc = _mm_or_ps(mask_acc, _mm_cmplt_ps(mstepacc, mevalue));
	}
	{
		__m128 mstepx = _mm_shuffle_ps(mstepx3, mstepx3, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 mstepy = _mm_shuffle_ps(mstepy3, mstepy3, _MM_SHUFFLE(1, 1, 1, 1));

		__m128 mrej2acc = _mm_shuffle_ps(mrej2acc3, mrej2acc3, _MM_SHUFFLE(1, 1, 1, 1));

		__m128 msteprej = _mm_add_ps(_mm_unpacklo_ps(_mm_setzero_ps(), mstepx), _mm_movelh_ps(_mm_setzero_ps(), mstepy));
		__m128 mstepacc = _mm_add_ps(msteprej, mrej2acc);

		__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(1, 1, 1, 1));

		mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
		mask_acc = _mm_or_ps(mask_acc, _mm_cmplt_ps(mstepacc, mevalue));
	}
	{
		__m128 mstepx = _mm_shuffle_ps(mstepx3, mstepx3, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 mstepy = _mm_shuffle_ps(mstepy3, mstepy3, _MM_SHUFFLE(2, 2, 2, 2));

		__m128 mrej2acc = _mm_shuffle_ps(mrej2acc3, mrej2acc3, _MM_SHUFFLE(2, 2, 2, 2));

		__m128 msteprej = _mm_add_ps(_mm_unpacklo_ps(_mm_setzero_ps(), mstepx), _mm_movelh_ps(_mm_setzero_ps(), mstepy));
		__m128 mstepacc = _mm_add_ps(msteprej, mrej2acc);

		__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(2, 2, 2, 2));

		mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
		mask_acc = _mm_or_ps(mask_acc, _mm_cmplt_ps(mstepacc, mevalue));
	}

	__m128i mix = _mm_add_epi32(_mm_set1_epi32(cur_region.x), _mm_unpacklo_epi32(_mm_setzero_si128(), mineww));
	__m128i miy = _mm_add_epi32(_mm_set1_epi32(cur_region.y), _mm_unpacklo_epi64(_mm_setzero_si128(), minewh));
	__m128i miregion = _mm_or_si128(_mm_or_si128(_mm_or_si128(mix, _mm_slli_epi32(miy, 8)), _mm_slli_epi32(mineww, 16)), _mm_slli_epi32(minewh, 24));
	__m128i mimask_acc = _mm_castps_si128(mask_acc);
	mimask_acc = _mm_andnot_si128(mimask_acc, _mm_set1_epi32(0x80000000));
	miregion = _mm_or_si128(miregion, mimask_acc);

	uint32_t region_code[4];
	_mm_storeu_si128(reinterpret_cast<__m128i*>(&region_code[0]), miregion);

	__m128 mx = _mm_cvtepi32_ps(mix);
	__m128 my = _mm_cvtepi32_ps(miy);
	mask_rej = _mm_or_ps(mask_rej, _mm_cmpge_ps(_mm_set1_ps(x_min), _mm_add_ps(mx, _mm_cvtepi32_ps(mineww))));
	mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(_mm_set1_ps(x_max), mx));
	mask_rej = _mm_or_ps(mask_rej, _mm_cmpge_ps(_mm_set1_ps(y_min), _mm_add_ps(my, _mm_cvtepi32_ps(minewh))));
	mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(_mm_set1_ps(y_max), my));

	int rejections = ~_mm_movemask_ps(mask_rej) & 0xF;
	unsigned long t;
	while (_BitScanForward(&t, rejections)){
		EFLIB_ASSERT(t < 4, "");

		test_regions[test_region_size] = region_code[t];
		++ test_region_size;

		rejections &= rejections - 1;
	}
#else
	float evalue[3];
	float step_x[3];
	float step_y[3];
	float rej_to_acc[3];
	for (int e = 0; e < 3; ++ e){
		evalue[e] = edge_factors[e].z - ((left + mark_x[e] * new_w) * edge_factors[e].x + (top + mark_y[e] * new_h) * edge_factors[e].y);
		step_x[e] = new_w * edge_factors[e].x;
		step_y[e] = new_h * edge_factors[e].y;
		rej_to_acc[e] = -abs(step_x[e]) - abs(step_y[e]);
	}

	for (int ty = 0; ty < 2; ++ ty){
		uint32_t y = cur_region.y + new_h * ty;
		for (int tx = 0; tx < 2; ++ tx){
			uint32_t x = cur_region.x + new_w * tx;

			if ((x_min < x + new_w) && (x_max >= x)
				&& (y_min < y + new_h) && (y_max >= y))
			{
				int rejection = 0;
				int acception = 1;

				// Trival rejection & acception
				for (int e = 0; e < 3; ++ e){
					float step = tx * step_x[e] + ty * step_y[e];
					rejection |= (step < evalue[e]);
					acception &= (step + rej_to_acc[e] >= evalue[e]);
				}

				if (!rejection){
					test_regions[test_region_size] = x + (y << 8) + (new_w << 16) + (new_h << 24) + (acception << 31);
					++ test_region_size;
				}
			}
		}
	}
#endif
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
void rasterizer::rasterize_triangle(uint32_t prim_id, uint32_t full, const vs_output& v0, const vs_output& v1, const vs_output& v2, const viewport& vp, const h_pixel_shader& pps)
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

#ifndef USE_TRADITIONAL_RASTERIZER
	const vec3* edge_factors = &edge_factors_[prim_id * 3];
	const bool mark_x[3] = {
		edge_factors[0].x > 0, edge_factors[1].x > 0, edge_factors[2].x > 0
	};
	const bool mark_y[3] = {
		edge_factors[0].y > 0, edge_factors[1].y > 0, edge_factors[2].y > 0
	};
	
	enum TRI_VS_TILE {
		TVT_FULL,
		TVT_PARTIAL,
		TVT_EMPTY,
		TVT_PIXEL
	};

	const vs_output projed_vert0 = project(v0);
	const vs_output projed_vert1 = project(v1);
	const vs_output projed_vert2 = project(v2);

	//初始化边及边上属性的差
	vs_output e01 = projed_vert1 - projed_vert0;
	vs_output e02 = projed_vert2 - projed_vert0;

	//计算面积
	float area = cross_prod2(e02.position.xy(), e01.position.xy());
	if(equal<float>(area, 0.0f)) return;
	float inv_area = 1.0f / area;

	/**********************************************************
	*  求解各个属性的差分式
	*********************************************************/
	vs_output ddx((e02 * e01.position.y - e02.position.y * e01)*inv_area);
	vs_output ddy((e01 * e02.position.x - e01.position.x * e02)*inv_area);

	triangle_info info;
	info.set(v0.position, ddx, ddy);
	pps->ptriangleinfo_ = &info;

	const float x_min = min(v0.position.x, min(v1.position.x, v2.position.x)) - vp.x;
	const float x_max = max(v0.position.x, max(v1.position.x, v2.position.x)) - vp.x;
	const float y_min = min(v0.position.y, min(v1.position.y, v2.position.y)) - vp.y;
	const float y_max = max(v0.position.y, max(v1.position.y, v2.position.y)) - vp.y;

	/*************************************************
	*   开始绘制多边形。
	*	The algorithm is from Larrabee
	*************************************************/

	const h_blend_shader& hbs = pparent_->get_blend_shader();
	const size_t num_samples = hfb_->get_num_samples();

	uint32_t test_regions[2][TILE_SIZE / 2 * TILE_SIZE / 2];
	uint32_t test_region_size[2] = { 0, 0 };
	test_regions[0][0] = (fast_floori(vp.w) << 16) + (fast_floori(vp.h) << 24) + (full << 31);
	test_region_size[0] = 1;
	int src_stage = 0;
	int dst_stage = !src_stage;

	uint32_t pixel_mask[TILE_SIZE * TILE_SIZE];
	uint8_t pixel_begin[TILE_SIZE];
	memset(pixel_begin, TILE_SIZE, sizeof(pixel_begin));
	uint8_t pixel_end[TILE_SIZE];
	memset(pixel_end, 0, sizeof(pixel_end));

	const uint32_t full_mask = (1UL << num_samples) - 1;

	const int vpleft0 = fast_floori(vp.x);
	const int vptop0 = fast_floori(vp.y);
	const int vpbottom0 = fast_floori(vp.y + vp.h);

	while (test_region_size[src_stage] > 0){
		test_region_size[dst_stage] = 0;

		for (size_t ivp = 0; ivp < test_region_size[src_stage]; ++ ivp){
			const uint32_t packed_region = test_regions[src_stage][ivp];
			eflib::rect<uint32_t> cur_region(packed_region & 0xFF, (packed_region >> 8) & 0xFF,
				(packed_region >> 16) & 0xFF, (packed_region >> 24) & 0x7F);
			TRI_VS_TILE intersect = (packed_region >> 31) ? TVT_FULL : TVT_PARTIAL;

			const int vpleft = fast_floori(max(0.0f, vp.x + cur_region.x));
			const int vptop = fast_floori(max(0.0f, vp.y + cur_region.y));
			const int vpright = fast_floori(min(vp.x + cur_region.x + cur_region.w, static_cast<float>(hfb_->get_width())));
			const int vpbottom = fast_floori(min(vp.y + cur_region.y + cur_region.h, static_cast<float>(hfb_->get_height())));

			// For one pixel region
			if ((TVT_PARTIAL == intersect) && (cur_region.w <= 2) && (cur_region.h <= 2)){
				intersect = TVT_PIXEL;
			}

			switch (intersect)
			{
			case TVT_EMPTY:
				break;

			case TVT_FULL: 
				this->draw_whole_tile(pixel_begin, pixel_end, pixel_mask, vpleft - vpleft0, vptop - vptop0, vpright - vpleft0, vpbottom - vptop0, full_mask);
				break;

			case TVT_PIXEL:
				this->draw_pixels(pixel_begin, pixel_end, pixel_mask, vpleft0, vptop0, vpleft, vptop, edge_factors, num_samples);
				break;

			default:
				this->subdivide_tile(vpleft, vptop, cur_region, edge_factors, mark_x, mark_y, test_regions[dst_stage], test_region_size[dst_stage],
					x_min, x_max, y_min, y_max);
				break;
			}
		}

		src_stage = (src_stage + 1) & 1;
		dst_stage = !src_stage;
	}

	const int y_begin = max(vptop0, fast_floori(y_min + vp.y));
	const int y_end = min(vpbottom0, fast_ceili(y_max + vp.y) + 1);

	const float offsetx = vpleft0 + 0.5f - v0.position.x;
	const float offsety = y_begin + 0.5f - v0.position.y;

	//设置基准扫描线的属性
	vs_output base_vert = projed_vert0;
	integral(base_vert, offsety, ddy);
	integral(base_vert, offsetx, ddx);

	for(int iy = y_begin; iy < y_end; ++iy)
	{
		//光栅化
		vs_output px_in(base_vert);
		ps_output px_out;
		vs_output unprojed;

		const int dy = iy - vptop0;
		if (pixel_end[dy] > pixel_begin[dy])
		{
			integral(px_in, static_cast<float>(pixel_begin[dy]), ddx);

			for(int dx = pixel_begin[dy], end = pixel_end[dy]; dx < end; ++dx){
				uint32_t mask = pixel_mask[dy * TILE_SIZE + dx];

				unproject(unprojed, px_in);
				if(pps->execute(unprojed, px_out)){
					const int ix = dx + vpleft0;
					if (1 == num_samples){
						hfb_->render_sample(hbs, ix, iy, 0, px_out, px_in.position.z);
					}
					else{
						if (full_mask == mask){
							for (unsigned long i_sample = 0; i_sample < num_samples; ++ i_sample){
								const vec2& sp = samples_pattern_[i_sample];
								const float ddxz = (sp.x - 0.5f) * ddx.position.z;
								const float ddyz = (sp.y - 0.5f) * ddy.position.z;
								hfb_->render_sample(hbs, ix, iy, i_sample, px_out, px_in.position.z + ddxz + ddyz);
							}
						}
						else{
							unsigned long i_sample;
							while (_BitScanForward(&i_sample, mask)){
								const vec2& sp = samples_pattern_[i_sample];
								const float ddxz = (sp.x - 0.5f) * ddx.position.z;
								const float ddyz = (sp.y - 0.5f) * ddy.position.z;
								hfb_->render_sample(hbs, ix, iy, i_sample, px_out, px_in.position.z + ddxz + ddyz);

								mask &= mask - 1;
							}
						}
					}
				}

				integral(px_in, ddx);
			}
		}

		//差分递增
		integral(base_vert, ddy);
	}
#else
	UNREF_PARAM(full);

	struct scanline_info
	{
		size_t scanline_width;

		vs_output ddx;

		vs_output base_vert;
		size_t base_x;
		size_t base_y;

		scanline_info()
		{}

		scanline_info(const scanline_info& rhs)
			:ddx(rhs.ddx), base_vert(rhs.base_vert), scanline_width(scanline_width),
			base_x(rhs.base_x), base_y(rhs.base_y)
		{}

		scanline_info& operator = (const scanline_info& rhs){
			ddx = rhs.ddx;
			base_vert = rhs.base_vert;
			base_x = rhs.base_x;
			base_y = rhs.base_y;
		}
	};

		/**********************************************************
	*        将顶点按照y大小排序，求出三角形面积与边
	**********************************************************/
	const vs_output* pvert[3] = {&v0, &v1, &v2};

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

	//初始化dxdy
	float dxdy_01 = eflib::equal<float>(e01.position.y, 0.0f) ? 0.0f: e01.position.x / e01.position.y;
	float dxdy_02 = eflib::equal<float>(e02.position.y, 0.0f) ? 0.0f: e02.position.x / e02.position.y;
	float dxdy_12 = eflib::equal<float>(e12.position.y, 0.0f) ? 0.0f: e12.position.x / e12.position.y;

	//计算面积
	float area = cross_prod2(e02.position.xy(), e01.position.xy());
	if(equal<float>(area, 0.0f)) return;
	float inv_area = 1.0f / area;

	/**********************************************************
	*  求解各个属性的差分式
	*********************************************************/
	vs_output ddx((e02 * e01.position.y - e02.position.y * e01)*inv_area);
	vs_output ddy((e01 * e02.position.x - e01.position.x * e02)*inv_area);

	triangle_info info;
	info.set(v0.position, ddx, ddy);
	pps->ptriangleinfo_ = &info;

	/*************************************
	*   设置基本的scanline属性。
	*   这些属性将在多个扫描行中保持相同
	************************************/
	scanline_info base_scanline;
	base_scanline.ddx = ddx;

	/*************************************************
	*   开始绘制多边形。经典的上-下分割绘制算法
	*   对扫描线光栅化保证是自左向右的。
	*   所以不需要考虑major edge是在左或者右边。
	*************************************************/

	const int bot_part = 0;
	//const int top_part = 1;

	int vpleft = fast_floori(max(0.0f, vp.x));
	int vptop = fast_floori(max(0.0f, vp.y));
	int vpright = fast_floori(min(vp.x+vp.w, (float)(hfb_->get_width())));
	int vpbottom = fast_floori(min(vp.y+vp.h, (float)(hfb_->get_height())));

	const h_blend_shader& hbs = pparent_->get_blend_shader();

	for(int iPart = 0; iPart < 2; ++iPart){

		//两条边的dxdy
		float dxdy0 = 0.0f;
		float dxdy1 = 0.0f;

		//起始/终止的x与y坐标; 
		//到三角形基准点位移,用于计算顶点属性
		float
			fsx0(0.0f), fsx1(0.0f), // 基准点的起止x坐标
			fsy(0.0f), fey(0.0f),	// Part的起止扫描线y坐标
			fcx0(0.0f), fcx1(0.0f), // 单根扫描线的起止点坐标
			offsety(0.0f);

		int isy(0), iey(0);	//Part的起止扫描线号

		//扫描线的起止顶点
		const vs_output* s_vert = NULL;
		const vs_output* e_vert = NULL;

		//依据片段设置起始参数
		if(iPart == bot_part){
			s_vert = pvert[0];
			e_vert = pvert[1];

			dxdy0 = dxdy_01;
		} else {
			s_vert = pvert[1];
			e_vert = pvert[2];

			dxdy0 = dxdy_12;
		}
		dxdy1 = dxdy_02;

		if(equal<float>(s_vert->position.y, e_vert->position.y)){
			continue; // next part
		}

		fsy = fast_ceil(s_vert->position.y + 0.5f) - 1;
		fey = fast_ceil(e_vert->position.y - 0.5f) - 1;

		isy = fast_floori(fsy);
		iey = fast_floori(fey);

		offsety = fsy + 0.5f - pvert[0]->position.y;

		//起点的x计算由于三角形的不同而有所不同
		if(iPart == bot_part){
			fsx0 = pvert[0]->position.x + dxdy_01*(fsy + 0.5f - pvert[0]->position.y);
		} else {
			fsx0 = pvert[1]->position.x + dxdy_12*(fsy + 0.5f - pvert[1]->position.y);
		}
		fsx1 = pvert[0]->position.x + dxdy_02*(fsy + 0.5f - pvert[0]->position.y);

		//设置基准扫描线的属性
		base_scanline.base_vert = projed_vert0;
		integral(base_scanline.base_vert, offsety, ddy);

		//当前的基准扫描线，起点在(base_vert.x, scanline.y)处。
		//在传递到rasterize_scanline之前需要将基础点调整到扫描线的最左端。
		scanline_info current_base_scanline(base_scanline);

		for(int iy = isy; iy <= iey; ++iy)
		{	
			//如果扫描线在view port的外面则跳过。	
			if( iy >= vpbottom ){
				break;
			}

			if( iy >= vptop ){
				//扫描线在视口内的就做扫描线
				int icx_s = 0;
				int icx_e = 0;

				fcx0 = dxdy0 * (iy - isy) + fsx0;
				fcx1 = dxdy1 * (iy - isy) + fsx1;

				//LOG: 记录扫描线的起止点。版本
				//if (fcx0 > 256.0 && iy == 222)
				//{
				//	log_serializer_indent_scope<log_system<slog_type>::slog_type> scope(&log_system<slog_type>::instance());
				//	log_system<slog_type>::instance().write(
				//		to_tstring(str(format("%1%") % iy)), 
				//		to_tstring(str(format("%1$8.5f, %2$8.5f") % fcx0 % fcx1)), LOGLEVEL_MESSAGE
				//		);
				//}

				if(fcx0 < fcx1){
					icx_s = fast_ceili(fcx0 + 0.5f) - 1;
					icx_e = fast_ceili(fcx1 - 0.5f) - 1;
				} else {
					icx_s = fast_ceili(fcx1 + 0.5f) - 1;
					icx_e = fast_ceili(fcx0 - 0.5f) - 1;
				}

				//如果起点大于终点说明scanline中不包含任何像素中心，直接跳过。
				if ((icx_s <= icx_e) && (icx_s < vpright) && (icx_e >= vpleft)) {
					icx_s = eflib::clamp(icx_s, vpleft, vpright - 1);
					icx_e = eflib::clamp(icx_e, vpleft, vpright - 1);

					float offsetx = icx_s + 0.5f - pvert[0]->position.x;

					//设置扫描线信息
					scanline_info scanline(current_base_scanline);
					integral(scanline.base_vert, offsetx, ddx);

					scanline.base_x = icx_s;
					scanline.base_y = iy;
					scanline.scanline_width = icx_e - icx_s + 1;

					//光栅化
					vec3* edge_factors = &edge_factors_[prim_id * 3];

					vs_output px_in(scanline.base_vert);
					ps_output px_out;
					vs_output unprojed;
					for(size_t i_pixel = 0; i_pixel < scanline.scanline_width; ++i_pixel)
					{
						unproject(unprojed, px_in);
						if(pps->execute(unprojed, px_out)){
							const size_t num_samples = hfb_->get_num_samples();
							if (1 == num_samples){
								hfb_->render_sample(hbs, scanline.base_x + i_pixel, scanline.base_y, 0, px_out, px_out.depth);
							}
							else{
								for (int i_sample = 0; i_sample < num_samples; ++ i_sample){
									const vec2& sp = samples_pattern_[i_sample];
									bool intersect = true;
									for (int e = 0; intersect && (e < 3); ++ e){
										if ((scanline.base_x + i_pixel + sp.x) * edge_factors[e].x
												- (scanline.base_y + sp.y) * edge_factors[e].y
												- edge_factors[e].z > 0){
											intersect = false;
										}
									}
									float samples_depth;
									if (intersect){
										float ddx = (sp.x - 0.5f) * pps->get_pos_ddx().w;
										float ddy = (sp.y - 0.5f) * pps->get_pos_ddy().w;
										float ddxz = ddx * pps->get_pos_ddx().z;
										float ddyz = ddy * pps->get_pos_ddy().z;
										samples_depth = (px_in.position.z * px_in.position.w + std::sqrt(ddxz * ddxz + ddyz * ddyz))
											/ (px_in.position.w + std::sqrt(ddx * ddx + ddy * ddy));
									}
									else{
										samples_depth = 1;
									}

									hfb_->render_sample(hbs, scanline.base_x + i_pixel, scanline.base_y, i_sample, px_out, samples_depth);
								}
							}
						}

						integral(px_in, scanline.ddx);
					}
				}
			}

			//差分递增
			integral(current_base_scanline.base_vert, ddy);
		}
	}
#endif
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

void rasterizer::geometry_setup_func(uint32_t* num_clipped_verts, vs_output* clipped_verts, uint32_t* cliped_indices,
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
		EFLIB_ASSERT(false, "枚举值无效：无效的Primitive Topology");
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
					const float inv_abs_w = 1 / abs(v.position.w);
					const float x = v.position.x * inv_abs_w;
					const float y = v.position.y * inv_abs_w;
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

void rasterizer::dispatch_primitive_func(std::vector<std::vector<uint32_t> >& tiles,
		const uint32_t* clipped_indices, const vs_output* clipped_verts_full, int32_t prim_count, uint32_t stride, atomic<int32_t>& working_package, int32_t package_size){

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
			const vec4* pv[3];
			for (size_t j = 0; j < stride; ++ j){
				pv[j] = &clipped_verts_full[clipped_indices[i * stride + j]].position;
			}

			if (3 == stride){
				// x * (y1 - y0) - y * (x1 - x0) - (y1 * x0 - x1 * y0)
				vec3* edge_factors = &edge_factors_[i * 3];
				for (int e = 0; e < 3; ++ e){
					const int se = e;
					const int ee = (e + 1) % 3;
					edge_factors[e].x = pv[se]->y - pv[ee]->y;
					edge_factors[e].y = pv[ee]->x - pv[se]->x;
					edge_factors[e].z = pv[ee]->x * pv[se]->y - pv[ee]->y * pv[se]->x;
				}
			}

			x_min = pv[0]->x;
			x_max = pv[0]->x;
			y_min = pv[0]->y;
			y_max = pv[0]->y;
			for (size_t j = 1; j < stride; ++ j){
				x_min = min(x_min, pv[j]->x);
				x_max = max(x_max, pv[j]->x);
				y_min = min(y_min, pv[j]->y);
				y_max = max(y_max, pv[j]->y);
			}

			const int sx = std::min(fast_floori(std::max(0.0f, x_min) / TILE_SIZE), num_tiles_x);
			const int sy = std::min(fast_floori(std::max(0.0f, y_min) / TILE_SIZE), num_tiles_y);
			const int ex = std::min(fast_ceili(std::max(0.0f, x_max) / TILE_SIZE) + 1, num_tiles_x);
			const int ey = std::min(fast_ceili(std::max(0.0f, y_max) / TILE_SIZE) + 1, num_tiles_y);
			if ((sx + 1 == ex) && (sy + 1 == ey)){
				// Small primitive
				tiles[sy * num_tiles_x + sx].push_back(i << 1);
			}
			else{
				if (3 == stride){
					vec3* edge_factors = &edge_factors_[i * 3];

					const bool mark_x[3] = {
						edge_factors[0].x > 0, edge_factors[1].x > 0, edge_factors[2].x > 0
					};
					const bool mark_y[3] = {
						edge_factors[0].y > 0, edge_factors[1].y > 0, edge_factors[2].y > 0
					};
					
					float step_x[3];
					float step_y[3];
					float rej_to_acc[3];
					for (int e = 0; e < 3; ++ e){
						step_x[e] = TILE_SIZE * edge_factors[e].x;
						step_y[e] = TILE_SIZE * edge_factors[e].y;
						rej_to_acc[e] = -abs(step_x[e]) - abs(step_y[e]);
					}

					for (int y = sy; y < ey; ++ y){
						for (int x = sx; x < ex; ++ x){
							int rejection = 0;
							int acception = 1;

							// Trival rejection & acception
							for (int e = 0; e < 3; ++ e){
								float evalue = edge_factors[e].z - ((x + mark_x[e]) * TILE_SIZE * edge_factors[e].x + (y + mark_y[e]) * TILE_SIZE * edge_factors[e].y);
								rejection |= (0 < evalue);
								acception &= (rej_to_acc[e] >= evalue);
							}

							if (!rejection){
								tiles[y * num_tiles_x + x].push_back((i << 1) | acception);
							}
						}
					}
				}
				else{
					for (int y = sy; y < ey; ++ y){
						for (int x = sx; x < ex; ++ x){
							tiles[y * num_tiles_x + x].push_back(i << 1);
						}
					}
				}
			}
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::rasterize_primitive_func(std::vector<std::vector<std::vector<uint32_t> > >& thread_tiles, int num_tiles_x,
		const uint32_t* clipped_indices, const vs_output* clipped_verts_full, const h_pixel_shader& pps, atomic<int32_t>& working_package, int32_t package_size)
{
	const int32_t num_tiles = static_cast<int32_t>(thread_tiles[0].size());
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
			std::vector<uint32_t> prims;
			for (size_t j = 0; j < thread_tiles.size(); ++ j){
				prims.insert(prims.end(), thread_tiles[j][i].begin(), thread_tiles[j][i].end());
			}
			std::sort(prims.begin(), prims.end());

			int y = i / num_tiles_x;
			int x = i - y * num_tiles_x;
			tile_vp.x = static_cast<float>(x * TILE_SIZE);
			tile_vp.y = static_cast<float>(y * TILE_SIZE);

			rasterize_func_(this, clipped_indices, clipped_verts_full, prims, tile_vp, pps);
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::rasterize_line_func(const uint32_t* clipped_indices, const vs_output* clipped_verts_full, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps){
	for (std::vector<uint32_t>::const_iterator iter = sorted_prims.begin(); iter != sorted_prims.end(); ++ iter){
		uint32_t iprim = *iter >> 1;
		this->rasterize_line(iprim, clipped_verts_full[clipped_indices[iprim * 2 + 0]], clipped_verts_full[clipped_indices[iprim * 2 + 1]], tile_vp, pps);
	}
}

void rasterizer::rasterize_triangle_func(const uint32_t* clipped_indices, const vs_output* clipped_verts_full, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps){
	for (std::vector<uint32_t>::const_iterator iter = sorted_prims.begin(); iter != sorted_prims.end(); ++ iter){
		uint32_t iprim = *iter >> 1;
		uint32_t full = *iter & 1;
		this->rasterize_triangle(iprim, full, clipped_verts_full[clipped_indices[iprim * 3 + 0]], clipped_verts_full[clipped_indices[iprim * 3 + 1]], clipped_verts_full[clipped_indices[iprim * 3 + 2]], tile_vp, pps);
	}
}

void rasterizer::compact_clipped_verts_func(uint32_t* clipped_indices, const uint32_t* clipped_indices_full,
		const uint32_t* addresses, const uint32_t* num_clipped_verts, int32_t prim_count,
		atomic<int32_t>& working_package, int32_t package_size){
	const int32_t num_packages = (prim_count + package_size - 1) / package_size;
	
	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages)
	{
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(prim_count, start + package_size);
		for (int32_t i = start; i < end; ++ i){
			memcpy(&clipped_indices[addresses[i]], &clipped_indices_full[i * 12], num_clipped_verts[i] * sizeof(*clipped_indices));
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::draw(size_t prim_count){

	EFLIB_ASSERT(pparent_, "Renderer 指针为空！可能该对象没有经过正确的初始化！");
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
		EFLIB_ASSERT(false, "枚举值无效：无效的Primitive Topology");
		return;
	}

	const viewport& vp = pparent_->get_viewport();
	int num_tiles_x = static_cast<size_t>(vp.w + TILE_SIZE - 1) / TILE_SIZE;
	int num_tiles_y = static_cast<size_t>(vp.h + TILE_SIZE - 1) / TILE_SIZE;

	atomic<int32_t> working_package(0);
	size_t num_threads = num_available_threads();

	// Culling, Clipping, Geometry setup
	boost::shared_array<uint32_t> num_clipped_verts(new uint32_t[prim_count]);
	boost::shared_array<vs_output> clipped_verts_full(new vs_output[prim_count * 6]);
	boost::shared_array<uint32_t> clipped_indices_full(new uint32_t[prim_count * 12]);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		global_thread_pool().schedule(boost::bind(&rasterizer::geometry_setup_func, this, &num_clipped_verts[0],
			&clipped_verts_full[0], &clipped_indices_full[0], static_cast<int32_t>(prim_count), primtopo,
			boost::ref(working_package), GEOMETRY_SETUP_PACKAGE_SIZE));
	}
	geometry_setup_func(&num_clipped_verts[0], &clipped_verts_full[0], &clipped_indices_full[0],
		static_cast<int32_t>(prim_count), primtopo, boost::ref(working_package), GEOMETRY_SETUP_PACKAGE_SIZE);
	global_thread_pool().wait();

	boost::shared_array<uint32_t> addresses(new uint32_t[prim_count]);
	addresses[0] = 0;
	for (size_t i = 1; i < prim_count; ++ i){
		addresses[i] = addresses[i - 1] + num_clipped_verts[i - 1];
	}

	uint32_t num_clipped_indices = addresses[prim_count - 1] + num_clipped_verts[prim_count - 1];
	boost::shared_array<uint32_t> clipped_indices(new uint32_t[num_clipped_indices]);
	working_package = 0;
	for (size_t i = 0; i < num_threads - 1; ++ i){
		global_thread_pool().schedule(boost::bind(&rasterizer::compact_clipped_verts_func, this, &clipped_indices[0],
			&clipped_indices_full[0], &addresses[0],
			&num_clipped_verts[0], static_cast<int32_t>(prim_count), boost::ref(working_package), COMPACT_CLIPPED_VERTS_PACKAGE_SIZE));
	}
	compact_clipped_verts_func(&clipped_indices[0], &clipped_indices_full[0], &addresses[0],
			&num_clipped_verts[0], static_cast<int32_t>(prim_count), boost::ref(working_package), COMPACT_CLIPPED_VERTS_PACKAGE_SIZE);
	global_thread_pool().wait();

	std::vector<std::vector<std::vector<uint32_t> > > thread_tiles(num_threads);
	working_package = 0;
	edge_factors_.resize(num_clipped_indices / prim_size * 3);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		thread_tiles[i].resize(num_tiles_x * num_tiles_y);
		global_thread_pool().schedule(boost::bind(&rasterizer::dispatch_primitive_func, this, boost::ref(thread_tiles[i]),
			&clipped_indices[0], &clipped_verts_full[0], static_cast<int32_t>(num_clipped_indices / prim_size),
			prim_size, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE));
	}
	thread_tiles[num_threads - 1].resize(num_tiles_x * num_tiles_y);
	dispatch_primitive_func(boost::ref(thread_tiles[num_threads - 1]),
		&clipped_indices[0], &clipped_verts_full[0], static_cast<int32_t>(num_clipped_indices / prim_size),
		prim_size, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE);
	global_thread_pool().wait();

	working_package = 0;
	h_pixel_shader hps = pparent_->get_pixel_shader();
	std::vector<h_pixel_shader> ppps(num_threads - 1);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		// create pixel_shader clone per thread from hps
		ppps[i] = hps->create_clone();
		global_thread_pool().schedule(boost::bind(&rasterizer::rasterize_primitive_func, this, boost::ref(thread_tiles),
			num_tiles_x, &clipped_indices[0], &clipped_verts_full[0], ppps[i], boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE));
	}
	rasterize_primitive_func(boost::ref(thread_tiles), num_tiles_x, &clipped_indices[0], &clipped_verts_full[0], hps, boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE);
	global_thread_pool().wait();
	// destroy all pixel_shader clone
	for (size_t i = 0; i < num_threads - 1; ++ i){
		hps->destroy_clone(ppps[i]);
	}
}

END_NS_SOFTART()
