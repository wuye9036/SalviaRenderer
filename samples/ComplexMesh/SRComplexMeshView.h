// SRComplexMeshView.h : interface of the CSRComplexMeshView class
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
#include <fstream>
#include <string>
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

typedef std::vector<std::pair<std::string, std::string> > TextureSlotsType;
struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 emit;
	float opacity;
	float specular_level;
	float shininess;

	TextureSlotsType texture_slots;
};

void ReadShortString(std::istream& file, std::string& str)
{
	uint8_t len;
	file.read(reinterpret_cast<char*>(&len), sizeof(len));
	str.resize(len);
	file.read(reinterpret_cast<char*>(&str[0]), len * sizeof(str[0]));
}

h_mesh LoadModel(salviar::h_renderer hsr, std::string const & mesh_name)
{
	std::vector<Material> mtls;
	std::vector<std::string> mesh_names;

	mesh* pmesh = new mesh(hsr.get());

	enum
	{
		vertbufid,
		normbufid,
		idxbufid,
		id_count
	};

	pmesh->set_buffer_count(id_count);

	std::ifstream file(mesh_name.c_str(), std::ios::binary);
	uint32_t fourcc;
	file.read(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

	uint64_t original_len, len;
	file.read(reinterpret_cast<char*>(&original_len), sizeof(original_len));
	file.read(reinterpret_cast<char*>(&len), sizeof(len));

	uint32_t num_mtls;
	file.read(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
	uint32_t num_meshes;
	file.read(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
	uint32_t num_joints;
	file.read(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
	uint32_t num_kfs;
	file.read(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));

	mtls.resize(num_mtls);
	for (uint32_t mtl_index = 0; mtl_index < num_mtls; ++ mtl_index)
	{
		Material& mtl = mtls[mtl_index];
		file.read(reinterpret_cast<char*>(&mtl.ambient.x), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.ambient.y), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.ambient.z), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.diffuse.x), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.diffuse.y), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.diffuse.z), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.specular.x), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.specular.y), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.specular.z), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.emit.x), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.emit.y), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.emit.z), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.opacity), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.specular_level), sizeof(float));
		file.read(reinterpret_cast<char*>(&mtl.shininess), sizeof(float));

		uint32_t num_texs;
		file.read(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));

		for (uint32_t tex_index = 0; tex_index < num_texs; ++ tex_index)
		{
			std::string type, name;
			ReadShortString(file, type);
			ReadShortString(file, name);
			mtl.texture_slots.push_back(std::make_pair(type, name));
		}
	}

	mesh_names.resize(num_meshes);

	salviar::h_input_layout layout;
	for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
	{
		ReadShortString(file, mesh_names[mesh_index]);

		int32_t mtl_id;
		file.read(reinterpret_cast<char*>(&mtl_id), sizeof(mtl_id));

		uint32_t num_ves;
		file.read(reinterpret_cast<char*>(&num_ves), sizeof(num_ves));

		for (uint32_t ve_index = 0; ve_index < num_ves; ++ ve_index)
		{
			struct vertex_element
			{
				uint32_t usage;
				uint8_t usage_index;
				uint64_t format;
			};

			vertex_element ve;
			file.read(reinterpret_cast<char*>(&ve), sizeof(ve));
		}
		layout.push_back(salviar::input_element_desc(stream_0, 0, sizeof(vec3), input_float3,
				input_register_usage_position, input_reg_0));
		layout.push_back(salviar::input_element_desc(stream_1, 0, sizeof(vec3), input_float3,
				input_register_usage_attribute, input_reg_1));

		uint32_t num_vertices;
		file.read(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));

		uint32_t max_num_blend;
		file.read(reinterpret_cast<char*>(&max_num_blend), sizeof(max_num_blend));

		salviar::h_buffer verts = pmesh->create_buffer(vertbufid, sizeof(vec3) * num_vertices);
		salviar::h_buffer normals = pmesh->create_buffer(normbufid, sizeof(vec3) * num_vertices);
		vec4* pverts = (vec4*)(verts->raw_data(0));
		vec3* pnorms = (vec3*)(normals->raw_data(0));

		file.read(reinterpret_cast<char*>(pverts), num_vertices * sizeof(vec3));
		file.read(reinterpret_cast<char*>(pnorms), num_vertices * sizeof(vec3));

		uint32_t num_triangles;
		file.read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));

		salviar::h_buffer indices = pmesh->create_buffer(idxbufid, sizeof(uint16_t) * num_triangles * 3);
		uint16_t* pidxs = (uint16_t*)(indices->raw_data(0));

		char is_index_16_bit;
		file.read(&is_index_16_bit, sizeof(is_index_16_bit));
		pmesh->set_index_type(is_index_16_bit ? index_int16 : index_int32);

		file.read(reinterpret_cast<char*>(pidxs), sizeof(uint16_t) * num_triangles * 3);

		pmesh->set_primitive_count(num_triangles);
		pmesh->set_index_buf_id(idxbufid);
		pmesh->set_default_layout(layout);
	}

	return h_mesh(pmesh);
}

class vs_mesh : public vertex_shader
{
	mat44 wv;
	mat44 proj;
	vec3 light_pos;
	vec3 eye_pos;
public:
	vs_mesh():wv(mat44::identity()), proj(mat44::identity()){
		register_var(_T("WorldViewMat"), wv);
		register_var(_T("ProjMat"), proj);
		register_var(_T("LightPos"), light_pos);
		register_var(_T("EyePos"), eye_pos);
	}

	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attributes[0];
		vec4 pos_es, normal_es;
		transform(pos_es, pos, wv);
		transform33(normal_es, in.attributes[1], wv);
		transform(out.position, pos_es, proj);
		out.attributes[0] = vec4(light_pos - pos_es.xyz(), 1);
		out.attributes[1] = vec4(eye_pos - pos_es.xyz(), 1);
		out.attributes[2] = normal_es;
	}

	uint32_t num_output_attributes() const
	{
		return 3;
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

class ps_mesh : public pixel_shader
{
public:
	ps_mesh()
	{
	}
	bool shader_prog(const vs_output& in, ps_output& out)
	{
		vec3 l = normalize3(in.attributes[0].xyz());
		vec3 e = normalize3(in.attributes[1].xyz());
		vec3 n = normalize3(in.attributes[2].xyz());
		float n_dot_l = dot_prod3(n, l);
		float roughness = 5;
		float spec = (roughness + 2) / 2 * pow(max(dot_prod3(normalize3(l + e), n), 0.0f), roughness);
		float clr = n_dot_l * (0.8f + spec * 0.4f);
		out.color[0] = vec4(clr, clr, clr, 1);
		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new ps_mesh(*this));
	}
	virtual void destroy_clone(h_pixel_shader& ps_clone)
	{
		ps_clone.reset();
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

class CSRComplexMeshView : public CWindowImpl<CSRComplexMeshView>
{
public:
	h_device present_dev;
	h_renderer hsr;

	h_mesh complex_mesh;

	h_vertex_shader pvs_mesh;
	h_pixel_shader pps_mesh;
	h_blend_shader pbs_mesh;

	h_rasterizer_state rs_back;

	h_surface display_surf;
	surface* pdsurf;

	uint32_t num_frames;
	float accumulate_time;
	float fps;

	Timer timer;

	CSRComplexMeshView::CSRComplexMeshView()
	{
	}

	CSRComplexMeshView::~CSRComplexMeshView()
	{
	}

	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP(CSRComplexMeshView)
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

		complex_mesh = LoadModel(hsr, "../../resources/M134 Predator.MESHML.model_bin");
		pvs_mesh.reset(new vs_mesh());
		pps_mesh.reset(new ps_mesh());
		pbs_mesh.reset(new ts_blend_off);

		rasterizer_desc rs_desc;
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

		static float s_angle = -1;
		//s_angle -= elapsed_time * 60.0f * (static_cast<float>(TWO_PI) / 360.0f);
		vec3 camera(cos(s_angle) * 400.0f, 600.0f, sin(s_angle) * 400.0f);

		mat44 world(mat44::identity()), view, proj, wv;
		
		vec3 eye(0.0f, 0.0f, 0.0f);
		mat_lookat(view, camera, eye, vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);

		for(float i = 0 ; i < 1 ; i ++)
		{
			mat_identity(world);
			mat_mul(wv, world, view);

			hsr->set_rasterizer_state(rs_back);
			pvs_mesh->set_constant(_T("WorldViewMat"), &wv);
			pvs_mesh->set_constant(_T("ProjMat"), &proj);
			vec3 light_pos(vec3(-4, 2, 0));
			pvs_mesh->set_constant(_T("LightPos"), &light_pos);
			pvs_mesh->set_constant(_T("EyePos"), &eye);

			hsr->set_vertex_shader(pvs_mesh);
			hsr->set_pixel_shader(pps_mesh);
			hsr->set_blend_shader(pbs_mesh);
			complex_mesh->render();
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
