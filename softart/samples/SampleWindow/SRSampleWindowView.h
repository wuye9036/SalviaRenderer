// SRSampleWindowView.h : interface of the CSRSampleWindowView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "softartx/include/presenter/WindowsUtil.h"
#include "softartx/include/presenter/dev.h"
#include "softartx/include/resource/mesh_io.h"

#include "softart/include/shader.h"
#include "softart/include/renderer_impl.h"
#include "softart/include/resource_manager.h"
#include "softart/include/rasterizer.h"

#include "eflib/include/eflib.h"

#include <iostream>
#include <boost/assign.hpp>
#include <boost/timer.hpp>

using namespace efl;
using namespace boost;
using namespace boost::assign;
using namespace std;

struct vert
{
	vec4 pos;
};

class vs : public vertex_shader
{
	mat44 wvp;
public:
	vs():wvp(mat44::identity()){
		register_var(_T("WorldViewProjMat"), wvp);
	}

	vs(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in[0];
		transform(out.position, wvp, pos);
		out.attributes[0] = in[0];//(vec4(1.0f, 1.0f, 1.0f, 1.0f) - in[0]);
		out.num_used_attribute = 1;
	}
};

class ps : public pixel_shader
{
public:
	bool shader_prog(const vs_output& in, ps_output& out)
	{
		out.color[0].xyz(in.attributes[0].xyz());
		out.color[0].w = 1.0f;

		return true;
	}
};

class ts : public blend_shader
{
public:
	bool shader_prog(backbuffer_pixel_out& inout, const backbuffer_pixel_in& in){
		if(inout.depth() > in.depth()){
			inout.color(0, in.color(0));
			inout.depth() = in.depth();
		}
		return true;
	}
};

class CSRSampleWindowView : public CWindowImpl<CSRSampleWindowView>
{
public:
	WindowsUtil_GDIPlus render;
	h_renderer hsr;
	h_texture sm_tex;

	h_mesh planar_mesh;
	h_mesh box_mesh;

	uint32_t num_frames;
	double accumulate_time;
	float fps;

	boost::timer timer;

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
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		renderer_parameters render_params = {0};
		render_params.backbuffer_format = pixel_format_color_bgra8;
		render_params.backbuffer_height = 512;
		render_params.backbuffer_width = 512;

		hsr = create_software_renderer(&render_params, h_device());

		planar_mesh = create_planar(
			hsr.get(), 
			vec3(-3.0f, -1.0f, -3.0f), 
			vec3(6.0f, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 6.0f),
			1, 1, true
			);
		
		box_mesh = create_box(hsr.get());

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		//TODO: Add your drawing code here

		RECT rc;
		this->GetClientRect(&rc);
		//hsr->get_device()->present(rect<size_t>(0, 0, 0, 0), rect<size_t>(0, 0, 0, 0));
		render.Render(dc.m_hDC, rc);
		return 0;
	}

	void Render()
	{
		// measure statistics
		++ num_frames;
		accumulate_time += static_cast<float>(timer.elapsed());

		// check if new second
		if (accumulate_time > 1)
		{
			// new second - not 100% precise
			fps = num_frames / accumulate_time;

			accumulate_time = 0;
			num_frames  = 0;
		}

		timer.restart();

		cout << fps << endl;

		hsr->clear_color(0, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		hsr->clear_depth(1.0f);

		vs* pvs = new vs();
		ps* pps = new ps();
		ts* pts = new ts();

		vec4 camera(1.5f, 1.5f, 1.5f, 1.0f);

		mat44 world(mat44::identity()), view, proj, wvp;
		
		mat_lookat(view, camera, vec4::gen_coord(0.0f, 0.0f, 0.0f), vec4::gen_vector(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, float(PI) / 2.0f, 1.0f, 0.1f, 100.0f);
		mat_mul(wvp, mat_mul(wvp, proj, view), world);

		pvs->set_constant(_T("WorldViewProjMat"), &wvp);

		hsr->set_vertex_shader(h_vertex_shader(pvs));
		hsr->set_pixel_shader(h_pixel_shader(pps));
		hsr->set_blend_shader(h_blend_shader(pts));

		hsr->set_cull_mode(cull_none);

		box_mesh->render();

		render.UpdateBackBuffer(static_pointer_cast<renderer_impl>(hsr)->get_framebuffer().get());

		InvalidateRect(NULL);
	}

	LRESULT OnClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		framebuffer* pfb = static_pointer_cast<renderer_impl>(hsr)->get_framebuffer().get();
		PPOINTS pp = (PPOINTS)(&lParam);
		if(pfb && size_t(pp->x) < pfb->get_width() && size_t(pp->y) < pfb->get_height())
		{
			color_rgba32f c = pfb->get_render_target(render_target_color, 0)->get_texel(pp->x, pfb->get_height() - 1 - pp->y);
			TCHAR str[512];
			_stprintf(str, _T("Pos: %3d, %3d, Color: %8.6f,%8.6f,%8.6f"), pp->x, pp->y, c.r, c.g, c.b);
			this->GetParent().SetWindowText(str);
		}
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 1;
    }
};
