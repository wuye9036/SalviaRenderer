// ColorizedTriangleView.h : interface of the CColorizedTriangleView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <salviar/include/presenter_dev.h>
#include <salviar/include/shader.h>
#include <salviar/include/shader_code.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/resource/mesh/sa/mesh_io.h>

#include "Timer.h"

#define PRESENTER_NAME "d3d9"

using namespace eflib;
using namespace salviar;
using namespace salviax;
using namespace salviax::resource;

using boost::shared_ptr;
using boost::static_pointer_cast;

using std::cout;
using std::endl;

char const* vs_code = 
"float4x4	wvpMatrix; \r\n"
"float4		lightPos; \r\n"
"struct VSIn{ \r\n"
"	float4 pos: POSITION; \r\n"
//"	float4 norm: NORMAL; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: SV_Position; \r\n"
//"	float4 norm: TEXCOORD(0); \r\n"
//"	float4 lightDir: TEXCOORD(1); \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
//"	out.norm = in.norm; \r\n"
"	out.pos = mul(in.pos, wvpMatrix); \r\n"
//"	out.lightDir = lightPos - in.pos;"
"	return out; \r\n"
"} \r\n"
;

class ps : public pixel_shader
{
public:

	ps()
	{
	}
	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		out.color[0] = color_rgba32f(0.8f, 0.9f, 0.9f, 1.0f ).get_vec4();
		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new ps(*this));
	}
	virtual void destroy_clone(h_pixel_shader& ps_clone)
	{
		ps_clone.reset();
	}
};

class bs : public blend_shader
{
public:
	bool shader_prog(size_t sample, backbuffer_pixel_out& inout, const ps_output& in)
	{
		color_rgba32f color(in.color[0]);
		inout.color( 0, sample, color_rgba32f(in.color[0]) );
		return true;
	}
};

class CColorizedTriangleView : public CWindowImpl<CColorizedTriangleView>
{
public:
	h_device present_dev;
	h_renderer hsr;
	h_texture sm_tex;

	h_mesh planar_mesh;

	h_pixel_shader pps;
	h_blend_shader pbs;

	h_rasterizer_state rs_back;

	h_surface display_surf;
	surface* pdsurf;

	uint32_t num_frames;
	float accumulate_time;
	float fps;

	Timer timer;

	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP(CColorizedTriangleView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnClick)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		std::_tstring dll_name = TEXT("salviax_");
		dll_name += TEXT(PRESENTER_NAME);
		dll_name += TEXT("_presenter");
#ifdef EFLIB_DEBUG
		dll_name += TEXT("_d");
#endif
		dll_name += TEXT(".dll");

		HMODULE presenter_dll = LoadLibrary(dll_name.c_str());
		typedef void (*create_presenter_device_func)(salviar::h_device& dev, void* param);
		create_presenter_device_func presenter_func = (create_presenter_device_func)GetProcAddress(presenter_dll, "salviax_create_presenter_device");
		presenter_func(present_dev, static_cast<void*>(m_hWnd));

		renderer_parameters render_params = {0};
		render_params.backbuffer_format = pixel_format_color_bgra8;
		render_params.backbuffer_height = 512;
		render_params.backbuffer_width = 512;
		render_params.backbuffer_num_samples = 1;

		hsr = create_software_renderer(&render_params, present_dev);

		const h_framebuffer& fb = hsr->get_framebuffer();
		if (fb->get_num_samples() > 1){
			display_surf.reset(new surface(fb->get_width(),
				fb->get_height(), 1, fb->get_buffer_format()));
			pdsurf = display_surf.get();
		}
		else{
			display_surf.reset();
			pdsurf = fb->get_render_target(render_target_color, 0);
		}

		rasterizer_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new rasterizer_state(rs_desc));

		shared_ptr<shader_code> compiled_code;
		salvia_create_shader( compiled_code, vs_code, lang_vertex_sl );

		hsr->set_vertex_shader_code( compiled_code );

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		planar_mesh = create_planar(
			hsr.get(), 
			vec3(-3.0f, -1.0f, -3.0f), 
			vec3(6, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 6),
			1, 1, true
			);

		pps.reset( new ps() );
		pbs.reset( new bs() );
		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		//TODO: Add your drawing code here

		present_dev->present(*pdsurf);
		return 0;
	}

	void Render()
	{
		// measure statistics
		++ num_frames;
		float elapsed_time = static_cast<float>(timer.elapsed());
		accumulate_time += elapsed_time;

		// check if new second
		if (accumulate_time > 1)
		{
			// new second - not 100% precise
			fps = num_frames / accumulate_time;

			accumulate_time = 0;
			num_frames  = 0;

			cout << fps << endl;
		}

		timer.restart();

		hsr->clear_color(0, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		hsr->clear_depth(1.0f);

		static float s_angle = 0;
		s_angle -= elapsed_time * 60.0f * (static_cast<float>(TWO_PI) / 360.0f);
		vec3 camera(cos(s_angle) * 1.5f, 1.5f, sin(s_angle) * 1.5f);

		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 100.0f);

		vec4 lightPos( sin(s_angle * 1.3f) * 0.7f, 0.6f, cos(s_angle * 1.3f) * 0.7f, 0.0f );

		for(float i = 0 ; i < 1 ; i ++)
		{
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			hsr->set_rasterizer_state(rs_back);

			hsr->set_vs_variable( "wvpMatrix", &wvp );
			hsr->set_vs_variable( "lightPos", &lightPos );

			hsr->set_pixel_shader(pps);
			hsr->set_blend_shader(pbs);
			planar_mesh->render();
		}

		if (hsr->get_framebuffer()->get_num_samples() > 1){
			hsr->get_framebuffer()->get_render_target(render_target_color, 0)->resolve(*display_surf);
		}

		InvalidateRect(NULL);
	}

	LRESULT OnClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		framebuffer* pfb = static_pointer_cast<renderer_impl>(hsr)->get_framebuffer().get();
		PPOINTS pp = (PPOINTS)(&lParam);
		if(pfb && size_t(pp->x) < pfb->get_width() && size_t(pp->y) < pfb->get_height())
		{
			color_rgba32f c = pfb->get_render_target(render_target_color, 0)->get_texel(pp->x, pfb->get_height() - 1 - pp->y, 0);
			TCHAR str[512];
#ifdef EFLIB_MSVC
			_stprintf_s(str, sizeof(str) / sizeof(str[0]), _T("Pos: %3d, %3d, Color: %8.6f,%8.6f,%8.6f"), pp->x, pp->y, c.r, c.g, c.b);
#else
			_stprintf(str, _T("Pos: %3d, %3d, Color: %8.6f,%8.6f,%8.6f"), pp->x, pp->y, c.r, c.g, c.b);
#endif
			this->GetParent().SetWindowText(str);
		}
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		{
			MSG msg;
			PeekMessage(&msg, m_hWnd, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE);
		}
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}
};
