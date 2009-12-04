#include "../include/rasterizer.h"

#include "../include/shaderregs_op.h"
#include "../include/clipper.h"
#include "../include/framebuffer.h"
#include "../include/renderer_impl.h"

#include "eflib/include/slog.h"

#include <algorithm>
#include <boost/format.hpp>

using namespace std;
using namespace efl;
using namespace boost;

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

//inherited
void rasterizer::initialize(renderer_impl* pparent)
{
	pparent_ = pparent;
	hfb_ = pparent->get_framebuffer();
	hps_ = pparent->get_pixel_shader();
}

IMPL_RS_UPDATED(rasterizer, pixel_shader)
{
	hps_ = pparent_->get_pixel_shader();
	return result::ok;
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
void rasterizer::rasterize_line_impl(const vs_output& v0, const vs_output& v1)
{
	vs_output diff = project(v1) - project(v0);
	const efl::vec4& dir = diff.wpos;
	float diff_dir = abs(dir.x) > abs(dir.y) ? dir.x : dir.y;

	//构造差分
	vs_output derivation = diff;

	vs_output ddx = diff * (diff.wpos.x / (diff.wpos.xy().length_sqr()));
	vs_output ddy = diff * (diff.wpos.y / (diff.wpos.xy().length_sqr()));

	ps_output px_out;

	//分为x major和y major使用DDA绘制线
	if( abs(dir.x) > abs(dir.y))
	{

		//调换起终点，使方向递增
		const vs_output *start, *end;
		if(dir.x < 0){
			start = &v1;
			end = &v0;
		} else {
			start = &v0;
			end = &v1;
		}

		triangle_info info;
		info.set(start->wpos, ddx, ddy);
		hps_->ptriangleinfo_ = &info;

		float fsx = floor(start->wpos.x + 0.5f);

		int sx = int(fsx);
		int ex = int(floor(end->wpos.x - 0.5f));

		//截取到屏幕内
		sx = efl::clamp<int>(sx, 0, int(hfb_->get_width() - 1));
		ex = efl::clamp<int>(ex, 0, int(hfb_->get_width()));

		//设置起点的vs_output
		vs_output px_start(project(*start));
		vs_output px_end(project(*end));
		float step = fsx + 0.5f - start->wpos.x;
		vs_output px_in = lerp(px_start, px_end, step / diff_dir);

		//x-major 的线绘制
		vs_output unprojed;
		for(int iPixel = sx; iPixel < ex; ++iPixel)
		{
			//忽略不在vp范围内的像素
			if(px_in.wpos.y >= hfb_->get_height()){
				if(dir.y > 0) break;
				continue;
			}
			if(px_in.wpos.y < 0){
				if(dir.y < 0) break;
				continue;
			}

			//进行像素渲染
			unproject(unprojed, px_in);
			if(hps_->execute(unprojed, px_out)){
				hfb_->render_pixel(iPixel, int(px_in.wpos.y), px_out);
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
		} else {
			start = &v0;
			end = &v1;
		}

		triangle_info info;
		info.set(start->wpos, ddx, ddy);
		hps_->ptriangleinfo_ = &info;

		float fsy = floor(start->wpos.y + 0.5f);

		int sy = int(fsy);
		int ey = int(floor(end->wpos.y - 0.5f));

		//截取到屏幕内
		sy = efl::clamp<int>(sy, 0, int(hfb_->get_height() - 1));
		ey = efl::clamp<int>(ey, 0, int(hfb_->get_height()));

		//设置起点的vs_output
		vs_output px_start(project(*start));
		vs_output px_end(project(*end));
		float step = fsy + 0.5f - start->wpos.y;
		vs_output px_in = lerp(px_start, px_end, (fsy + 0.5f - start->wpos.y) / diff_dir);

		//x-major 的线绘制
		vs_output unprojed;
		for(int iPixel = sy; iPixel < ey; ++iPixel)
		{
			//忽略不在vp范围内的像素
			if(px_in.wpos.x >= hfb_->get_width()){
				if(dir.x > 0) break;
				continue;
			}
			if(px_in.wpos.x < 0){
				if(dir.x < 0) break;
				continue;
			}

			//进行像素渲染
			unproject(unprojed, px_in);
			if(hps_->execute(unprojed, px_out)){
				hfb_->render_pixel(int(px_in.wpos.x), iPixel, px_out);
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
void rasterizer::rasterize_triangle_impl(const vs_output& v0, const vs_output& v1, const vs_output& v2)
{
	typedef slog<text_log_serializer> slog_type;
	log_serializer_indent_scope<log_system<slog_type>::slog_type> scope(&log_system<slog_type>::instance());

	//记录三角形的屏幕坐标系顶点。
	log_system<slog_type>::instance().write(_EFLIB_T("wv0"),
		to_tstring(str(format("( %1%, %2%, %3%)") % v0.wpos.x % v0.wpos.y % v0.wpos.z)), LOGLEVEL_MESSAGE
		);
	log_system<slog_type>::instance().write(_EFLIB_T("wv1"), 
		to_tstring(str(format("( %1%, %2%, %3%)") % v1.wpos.x % v1.wpos.y % v1.wpos.z)), LOGLEVEL_MESSAGE
		);
	log_system<slog_type>::instance().write(_EFLIB_T("wv2"), 
		to_tstring(str(format("( %1%, %2%, %3%)") % v2.wpos.x % v2.wpos.y % v2.wpos.z)), LOGLEVEL_MESSAGE
		);

	/**********************************************************
	*        将顶点按照y大小排序，求出三角形面积与边
	**********************************************************/
	const vs_output* pvert[3] = {&v0, &v1, &v2};

	//升序排列
	if(pvert[0]->wpos.y > pvert[1]->wpos.y){
		swap(pvert[1], pvert[0]);
	}
	if(pvert[1]->wpos.y > pvert[2]->wpos.y){
		swap(pvert[2], pvert[1]);
		if(pvert[0]->wpos.y > pvert[1]->wpos.y) 
			swap(pvert[1], pvert[0]);
	}

	//初始化边及边上属性的差
	vs_output e01 = project(*(pvert[1])) - project(*(pvert[0]));
	//float watch_x = e01.attributes[2].x;
	
	vs_output e02 = project(*(pvert[2])) - project(*(pvert[0]));
	vs_output e12;



	//初始化边上的各个分量差值。（只要算两条边就可以了。）
	e12.wpos = pvert[2]->wpos - pvert[1]->wpos;

	//初始化dxdy
	float dxdy_01 = efl::equal<float>(e01.wpos.y, 0.0f) ? 0.0f: e01.wpos.x / e01.wpos.y;
	float dxdy_02 = efl::equal<float>(e02.wpos.y, 0.0f) ? 0.0f: e02.wpos.x / e02.wpos.y;
	float dxdy_12 = efl::equal<float>(e12.wpos.y, 0.0f) ? 0.0f: e12.wpos.x / e12.wpos.y;

	//计算面积
	float area = cross_prod2(e02.wpos.xy(), e01.wpos.xy());
	float inv_area = 1.0f / area;
	if(equal<float>(area, 0.0f)) return;

	/**********************************************************
	*  求解各个属性的差分式
	*********************************************************/
	vs_output ddx((e02 * e01.wpos.y - e02.wpos.y * e01)*inv_area);
	vs_output ddy((e01 * e02.wpos.x - e01.wpos.x * e02)*inv_area);

	triangle_info info;
	info.set(pvert[0]->wpos, ddx, ddy);
	hps_->ptriangleinfo_ = &info;

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
			dxdy1 = dxdy_02;
		} else {
			s_vert = pvert[1];
			e_vert = pvert[2];

			dxdy0 = dxdy_12;
			dxdy1 = dxdy_02;
		}

		if(equal<float>(s_vert->wpos.y, e_vert->wpos.y)){
			continue; // next part
		}

		fsy = ceil(s_vert->wpos.y + 0.5f) - 1;
		fey = ceil(e_vert->wpos.y - 0.5f) - 1;

		isy = int(fsy);
		iey = int(fey);

		offsety = fsy + 0.5f - pvert[0]->wpos.y;

		//起点的x计算由于三角形的不同而有所不同
		if(iPart == bot_part){
			fsx0 = pvert[0]->wpos.x + dxdy_01*(fsy + 0.5f - pvert[0]->wpos.y);
			fsx1 = pvert[0]->wpos.x + dxdy_02*(fsy + 0.5f - pvert[0]->wpos.y);
		} else {
			fsx0 = pvert[1]->wpos.x + dxdy_12*(fsy +0.5f - pvert[1]->wpos.y);
			fsx1 = pvert[0]->wpos.x + dxdy_02*(fsy +0.5f - pvert[0]->wpos.y);
		}

		//设置基准扫描线的属性
		project(base_scanline.base_vert, *(pvert[0]));
		integral(base_scanline.base_vert, offsety, ddy);

		//当前的基准扫描线，起点在(base_vert.x, scanline.y)处。
		//在传递到rasterize_scanline之前需要将基础点调整到扫描线的最左端。
		scanline_info current_base_scanline(base_scanline);

		const viewport& vp = pparent_->get_viewport();

		int vpleft = int(max(0, vp.x));
		int vpbottom = int(max(0, vp.y));
		int vpright = int(min(vp.x+vp.w, hfb_->get_width()));
		int vptop = int(min(vp.y+vp.h, hfb_->get_height()));

		for(int iy = isy; iy <= iey; ++iy)
		{	
			//如果扫描线在view port的外面则跳过。	
			if( iy >= vptop ){
				break;
			}

			if( iy >= vpbottom ){
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
					icx_s = (int)ceil(fcx0 + 0.5f) - 1;
					icx_e = (int)ceil(fcx1 - 0.5f) - 1;
				} else {
					icx_s = (int)ceil(fcx1 + 0.5f) - 1;
					icx_e = (int)ceil(fcx0 - 0.5f) - 1;
				}

				icx_s = efl::clamp<int>(icx_s, vpleft, vpright - 1);
				icx_e = efl::clamp<int>(icx_e, vpleft, vpright - 1);

				//如果起点大于终点说明scanline中不包含任何像素中心，直接跳过。
				if(icx_s <= icx_e) {
					float offsetx = float(icx_s) + 0.5f - pvert[0]->wpos.x;

					//设置扫描线信息
					scanline_info scanline(current_base_scanline);
					integral(scanline.base_vert, offsetx, ddx);

					scanline.base_x = icx_s;
					scanline.base_y = iy;
					scanline.scanline_width = icx_e - icx_s + 1;

					//光栅化
					rasterize_scanline_impl(scanline);
				}
			}

			//差分递增
			integral(current_base_scanline.base_vert, 1.0f, ddy);
		}
	}
}

//扫描线光栅化程序，将对扫描线依据差分信息进行光栅化并将光栅化的片段传递到像素着色器中.
//Note:传入的像素将w乘回到attribute上.
void rasterizer::rasterize_scanline_impl(const scanline_info& sl)
{
	vs_output px_in(sl.base_vert);
	ps_output px_out;

	vs_output unprojed;
	for(size_t i_pixel = 0; i_pixel < sl.scanline_width; ++i_pixel)
	{
		//if(px_in.wpos.z <= 0.0f)
			//continue;

		//执行shader program
		unproject(unprojed, px_in);
		if(hps_->execute(unprojed, px_out)){
			hfb_->render_pixel(sl.base_x + i_pixel, sl.base_y, px_out);
		}

		integral(px_in, 1.0f, sl.ddx);
	}
}

rasterizer::rasterizer()
{
	cm_ = cull_back;
	fm_ = fill_solid;
}

void rasterizer::rasterize_line(const vs_output& v0, const vs_output& v1)
{
	//如果完全超过边界，则剔除
	const viewport& vp = pparent_->get_viewport();

	if(v0.wpos.x < 0 && v1.wpos.x < 0) return;
	if(v0.wpos.y < 0 && v1.wpos.y < 0) return;
	if(v0.wpos.z < vp.minz && v1.wpos.z < vp.minz) return;

	if(v0.wpos.x >= vp.w && v1.wpos.x >= vp.w) return;
	if(v0.wpos.y >= vp.w && v1.wpos.y >= vp.w) return;
	if(v0.wpos.z >= vp.maxz && v1.wpos.z >= vp.maxz) return;

	//render
	rasterize_line_impl(v0, v1);
}

void rasterizer::rasterize_triangle(const vs_output& v0, const vs_output& v1, const vs_output& v2)
{
	//边界剔除
	const viewport& vp = pparent_->get_viewport();
	
	//背面剔除
	if(cm_ != cull_none){
		float area = compute_area(v0, v1, v2);
		if( (cm_ == cull_front) && (area > 0) ){
			return;
		}
		if( (cm_ == cull_back) && (area < 0) ){
			return;
		}
	}

	//渲染
	if(fm_ == fill_wireframe){
		rasterize_line(v0, v1);
		rasterize_line(v1, v2);
		rasterize_line(v0, v2);
	} else {
		h_clipper clipper = pparent_->get_clipper();
		clipper->set_viewport(vp);
		const vector<const vs_output*>& clipped_verts = clipper->clip(v0, v1, v2);

		for(size_t i_tri = 1; i_tri < clipped_verts.size() - 1; ++i_tri){
			rasterize_triangle_impl(*clipped_verts[0], *clipped_verts[i_tri], *clipped_verts[i_tri+1]);
		}
		//rasterize_triangle_impl(v0, v1, v2);
	}
}

void rasterizer::set_cull_mode(cull_mode cm)
{
	cm_ = cm;
}

void rasterizer::set_fill_mode(fill_mode fm)
{
	fm_ = fm;
}