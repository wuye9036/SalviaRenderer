// SRSampleWindowView.h : interface of the CSRSampleWindowView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "salviax/include/resource/mesh/sa/mesh_io.h"
#include "salviax/include/resource/texture/gdiplus/tex_io_gdiplus.h"
#include "salviax/include/resource/texture/freeimage/tex_io_freeimage.h"
#include "salviar/include/presenter_dev.h"
#include "salviar/include/shader.h"
#include "salviar/include/renderer_impl.h"
#include "salviar/include/resource_manager.h"
#include "salviar/include/rasterizer.h"
#include <eflib/include/metaprog/util.h>
#include <iostream>
#include <boost/assign.hpp>
#include "Timer.h"

//#define PRESENTER_NAME "gdiplus"
#define PRESENTER_NAME "d3d9"
//#define PRESENTER_NAME "d3d11"

using namespace eflib;
using namespace boost;
using namespace boost::assign;
using namespace std;
using namespace salviar;
using namespace salviax;
using namespace salviax::resource;
using namespace Gdiplus;

struct vert
{
	vec4 pos;
};

class vs_box : public vertex_shader
{
	mat44 wvp;
public:
	vs_box():wvp(mat44::identity()){
		declare_constant(_T("WorldViewProjMat"), wvp);

		bind_semantic( "POSITION", 0, 0 );
		bind_semantic( "TEXCOORD", 0, 1 );
	}

	vs_box(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attributes[0];
		transform(out.position, pos, wvp);
		out.attributes[0] = in.attributes[0];//(vec4(1.0f, 1.0f, 1.0f, 1.0f) - in[0]);
		out.attributes[1] = in.attributes[1];
	}

	uint32_t num_output_attributes() const
	{
		return 2;
	}

	uint32_t output_attribute_modifiers(uint32_t index) const
	{
		switch (index)
		{
		case 0:
			return salviar::vs_output::am_linear;

		case 1:
			return salviar::vs_output::am_linear;

		case 2:
			return salviar::vs_output::am_linear;

		default:
			return salviar::vs_output::am_linear;
		}
	}
};

class ps_box : public pixel_shader
{
	salviar::h_sampler sampler_;
	salviar::h_texture tex_;
public:

	ps_box(const salviar::h_texture& tex)
		: tex_(tex)
	{
		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_linear;
		desc.addr_mode_u = address_clamp;
		desc.addr_mode_v = address_clamp;
		desc.addr_mode_w = address_clamp;
		sampler_.reset(new sampler(desc));
		sampler_->set_texture(tex_.get());
	}
	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f color = tex2d(*sampler_ , 1);
		color.a = 0.5;
		out.color[0] = color.get_vec4();

		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new ps_box(*this));
	}
	virtual void destroy_clone(h_pixel_shader& ps_clone)
	{
		ps_clone.reset();
	}
};

class vs_plane : public vertex_shader
{
	mat44 wvp;
public:
	vs_plane():wvp(mat44::identity()){
		declare_constant(_T("WorldViewProjMat"), wvp);
		bind_semantic( "POSITION", 0, 0 );
	}

	vs_plane(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attributes[0];
		transform(out.position, pos, wvp);
		out.attributes[0] = vec4(in.attributes[0].x, in.attributes[0].z, 0, 0);
	}

	uint32_t num_output_attributes() const
	{
		return 1;
	}

	uint32_t output_attribute_modifiers(uint32_t index) const
	{
		switch (index)
		{
		case 0:
			return salviar::vs_output::am_linear;

		default:
			return salviar::vs_output::am_linear;
		}
	}
};

class ps_plane : public pixel_shader
{
	salviar::h_sampler sampler_;
	salviar::h_texture tex_;
public:

	ps_plane(const salviar::h_texture& tex)
		: tex_(tex)
	{
		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_linear;
		sampler_.reset(new sampler(desc));
		sampler_->set_texture(tex_.get());
	}
	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f color = tex2d(*sampler_, 0);
		color.a = 1;
		out.color[0] = color.get_vec4();

		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new ps_plane(*this));
	}
	virtual void destroy_clone(h_pixel_shader& ps_clone)
	{
		ps_clone.reset();
	}
};

class ts_blend_on : public blend_shader
{
public:
	bool shader_prog(size_t sample, backbuffer_pixel_out& inout, const ps_output& in)
	{
		color_rgba32f color(in.color[0]);
		inout.color(0, sample, lerp(inout.color(0, sample), color, color.a));
		return true;
	}
};

class ts_blend_off : public blend_shader
{
public:
	bool shader_prog(size_t sample, backbuffer_pixel_out& inout, const ps_output& in)
	{
		inout.color(0, sample, color_rgba32f(in.color[0]));
		return true;
	}
};

class CSRSampleWindowView : public CWindowImpl<CSRSampleWindowView>
{
public:
	h_device present_dev;
	h_renderer hsr;
	h_texture sm_tex;

	h_mesh planar_mesh;
	h_mesh box_mesh;

	h_vertex_shader pvs_box;
	h_pixel_shader pps_box;
	h_vertex_shader pvs_plane;
	h_pixel_shader pps_plane;
	h_blend_shader pbs_box;
	h_blend_shader pbs_plane;

	h_rasterizer_state rs_front;
	h_rasterizer_state rs_back;

	h_surface display_surf;
	surface* pdsurf;

	uint32_t num_frames;
	float accumulate_time;
	float fps;

	Timer timer;

	CSRSampleWindowView::CSRSampleWindowView()
	{
	}

	CSRSampleWindowView::~CSRSampleWindowView()
	{
	}

	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP(CSRSampleWindowView)
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

		planar_mesh = create_planar(
			hsr.get(), 
			vec3(-3.0f, -1.0f, -3.0f), 
			vec3(6, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 6),
			1, 1, true
			);
		
		box_mesh = create_box(hsr.get());

		pvs_box.reset(new vs_box());
		pvs_plane.reset(new vs_plane());

		{
			h_texture tex = texture_io_fi::instance().load(hsr.get() , _T("../../resources/Dirt.jpg") , salviar::pixel_format_color_rgba8);
			tex->set_min_lod(8);
			tex->gen_mipmap(filter_linear);
			pps_box.reset(new ps_box(tex));
		}

		{
			h_texture tex = texture_io_fi::instance().load(hsr.get() , _T("../../resources/chessboard.png") , salviar::pixel_format_color_rgba8);
			tex->set_min_lod(5);
			tex->gen_mipmap(filter_linear);
			pps_plane.reset(new ps_plane(tex));
		}

		pbs_box.reset(new ts_blend_on);
		pbs_plane.reset(new ts_blend_off);

		rasterizer_desc rs_desc;
		rs_desc.cm = cull_front;
		rs_front.reset(new rasterizer_state(rs_desc));
		rs_desc.cm = cull_back;
		rs_back.reset(new rasterizer_state(rs_desc));

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

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

		for(float i = 0 ; i < 1 ; i ++)
		{
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			hsr->set_rasterizer_state(rs_back);
			pvs_plane->set_constant(_T("WorldViewProjMat"), &wvp);
			hsr->set_vertex_shader(pvs_plane);
			hsr->set_pixel_shader(pps_plane);
			hsr->set_blend_shader(pbs_plane);
			planar_mesh->render();
			
			hsr->set_rasterizer_state(rs_front);
			pvs_box->set_constant(_T("WorldViewProjMat"), &wvp);
			hsr->set_vertex_shader(pvs_box);
			hsr->set_pixel_shader(pps_box);
			hsr->set_blend_shader(pbs_box);
			box_mesh->render();

			hsr->set_rasterizer_state(rs_back);
			hsr->set_vertex_shader(pvs_box);
			hsr->set_pixel_shader(pps_box);
			hsr->set_blend_shader(pbs_box);
			box_mesh->render();
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

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		UNREF_PARAM(uMsg);
		UNREF_PARAM(wParam);
		UNREF_PARAM(lParam);
		UNREF_PARAM(bHandled);

		return 1;
    }
};
