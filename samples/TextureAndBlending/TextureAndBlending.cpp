#include <salvia/shader/shader.h>
#include <salvia/shader/shader_regs.h>
#include <salvia/shader/shader_object.h>
#include <salvia/core/sync_renderer.h>
#include <salvia/resource/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salvia/common/colors.h>

#include "salvia/ext/resource/mesh/mesh_io.h"
#include "salvia/ext/resource/mesh/mesh_io_obj.h"
#include <salvia/ext/resource/texture/tex_io.h>
#include <salvia/ext/swap_chain/swap_chain.h>

#include <salvia/utility/common/sample_app.h>
#include <salvia/utility/common/path.h>

#include <eflib/platform/main.h>

#include <vector>

using namespace eflib;
using namespace salvia::core;
using namespace salvia::ext;
using namespace salvia::ext::resource;
using namespace salviau;

using std::shared_ptr;
using std::dynamic_pointer_cast;
using std::string;
using std::vector;
using std::cout;
using std::endl;

#define SALVIA_ENABLE_PIXEL_SHADER 1

struct vert
{
	vec4 pos;
};

class vs_box : public cpp_vertex_shader
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
		vec4 pos = in.attribute(0);
		transform(out.position(), pos, wvp);
		out.attribute(0) = in.attribute(0);//(vec4(1.0f, 1.0f, 1.0f, 1.0f) - in[0]);
		out.attribute(1) = in.attribute(1);
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

    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}

};

char const* box_ps_code =
	"sampler samp;"
	"float4 ps_main( float4 pos: TEXCOORD0, float2 tex: TEXCOORD1 ): COLOR \r\n"
	"{\r\n"
	"	float4 color = tex2D(samp, tex); \r\n"
	"	color.w = 0.5f; \r\n"
	"	return color; \r\n"
	"}\r\n"
	;

char const* plane_ps_code =
	"sampler samp;"
	"float4 ps_main( float2 tex: TEXCOORD0 ): COLOR \r\n"
	"{\r\n"
	"	float4 color = tex2D(samp, tex); \r\n"
	"	color.w = 1.0f; \r\n" // color.w == color.a.
	"	return color; \r\n"
	"}\r\n"
	;
	

class ps_box : public cpp_pixel_shader
{
	sampler_ptr sampler_;

public:

	ps_box(sampler_ptr const& samp): sampler_(samp)
	{
	}

	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f color = tex2d(*sampler_ , 1);
		color.a = 0.5;
		out.color[0] = color.get_vec4();

		return true;
	}

    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}

};

class vs_plane : public cpp_vertex_shader
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
		vec4 pos = in.attribute(0);
		transform(out.position(), pos, wvp);
		out.attribute(0) = vec4(in.attribute(0).x(), in.attribute(0).z(), 0, 0);
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
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}

};

class ps_plane : public cpp_pixel_shader
{
	sampler_ptr sampler_;
public:

	ps_plane(sampler_ptr const& samp): sampler_(samp)
	{
	}

	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f color = tex2d(*sampler_, 0);
		color.a = 1;
		out.color[0] = color.get_vec4();

		return true;
	}

    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}
};

class ts_blend_on : public cpp_blend_shader
{
public:
	bool shader_prog(size_t sample, pixel_accessor& inout, const ps_output& in)
	{
		color_rgba32f color(in.color[0]);
		inout.color(0, sample, lerp(inout.color(0, sample), color, color.a));
		return true;
	}
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}

};

class ts_blend_off : public cpp_blend_shader
{
public:
	bool shader_prog(size_t sample, pixel_accessor& inout, const ps_output& in)
	{
		inout.color(0, sample, color_rgba32f(in.color[0]));
		return true;
	}
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}

};

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 3 : 1500;
int const TEST_FRAME_COUNT		= 5;

class texture_and_blending: public sample_app
{
public:
	texture_and_blending(): sample_app("TextureAndBlending")
	{
	}

protected:
	void on_init() override
	{
		create_devices_and_targets(data_->screen_width, data_->screen_height, 1, pixel_format_color_bgra8, pixel_format_color_rg32f);

		data_->renderer->set_viewport(data_->screen_vp);

		planar_mesh = create_planar(
			data_->renderer.get(), 
			vec3(-3.0f, -1.0f, -3.0f), 
			vec3(6, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 6),
			1, 1, true
			);
		
		box_mesh = create_box(data_->renderer.get());

		pvs_box.reset(new vs_box());
		pvs_plane.reset(new vs_plane());

		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_linear;
			desc.addr_mode_u = address_clamp;
			desc.addr_mode_v = address_clamp;
			desc.addr_mode_w = address_clamp;

			box_tex = load_texture(data_->renderer.get() , find_path(_EFLIB_T("texture_and_blending/Dirt.jpg")) , salviar::pixel_format_color_rgba8);
			box_tex->gen_mipmap(filter_linear, true);

			box_sampler = data_->renderer->create_sampler(desc, box_tex);
            pps_box.reset(new ps_box(box_sampler));

#ifdef SALVIA_ENABLE_PIXEL_SHADER
			cout << "Creating Box Pixel Shader ..." << endl;
			psc_box = compile( box_ps_code, lang_pixel_shader );
#endif
		}

		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_linear;

			plane_tex = load_texture(data_->renderer.get(), find_path(_EFLIB_T("texture_and_blending/chessboard.png")) , salviar::pixel_format_color_rgba8);
			plane_tex->gen_mipmap(filter_linear, true);
			
            plane_sampler = data_->renderer->create_sampler(desc, plane_tex);
            pps_plane.reset(new ps_plane(plane_sampler));

#ifdef SALVIA_ENABLE_PIXEL_SHADER
			cout << "Creating Plane Pixel Shader ..." << endl;
			psc_plane = compile( plane_ps_code, lang_pixel_shader );
#endif
		}

		pbs_box.reset(new ts_blend_on);
		pbs_plane.reset(new ts_blend_off);

		raster_desc rs_desc;
		rs_desc.cm = cull_front;
		rs_front.reset(new raster_state(rs_desc));
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

		switch(data_->mode)
		{
		case app_modes::benchmark:
			quit_at_frame(BENCHMARK_FRAME_COUNT);
			break;
		case app_modes::test:
			quit_at_frame(TEST_FRAME_COUNT);
			break;
		}
	}

	void on_frame() override
    {
		profiling("BackBufferClearing", [this](){
			data_->renderer->clear_color(data_->color_target, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
			data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth | clear_stencil, 1.0f, 0);
		});

		float scene_sec = 0.0f;
		switch(data_->mode)
		{
		case app_modes::benchmark:
			scene_sec = static_cast<float>(data_->frame_count * 6) / (BENCHMARK_FRAME_COUNT - 1);
			break;
		case app_modes::test:
			scene_sec = static_cast<float>(data_->frame_count * 6) / (TEST_FRAME_COUNT - 1);
			break;
		default:
			scene_sec = static_cast<float>(data_->total_elapsed_sec);
			break;
		}

		float angle = -scene_sec * 60.0f * (static_cast<float>(TWO_PI) / 360.0f);
		vec3 camera(cos(angle) * 1.5f, 1.5f, sin(angle) * 1.5f);

		mat44 world(mat44::identity()), view, proj, wvp;
		
		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), data_->screen_aspect_ratio, 0.1f, 100.0f);

		mat_translate(world , -0.5f, 0, -0.5f);
		mat_mul(wvp, world, mat_mul(wvp, view, proj));

		profiling("Rendering", [&](){
			data_->renderer->set_rasterizer_state(rs_back);
			pvs_plane->set_constant(_T("WorldViewProjMat"), &wvp);
			data_->renderer->set_vertex_shader(pvs_plane);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			data_->renderer->set_pixel_shader_code( psc_plane );
			data_->renderer->set_ps_sampler( "samp", plane_sampler );
#else
			data_->renderer->set_pixel_shader(pps_plane);
#endif
			data_->renderer->set_blend_shader(pbs_plane);
			planar_mesh->render();
			
			data_->renderer->set_rasterizer_state(rs_front);
			pvs_box->set_constant(_T("WorldViewProjMat"), &wvp);
			data_->renderer->set_vertex_shader(pvs_box);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			data_->renderer->set_pixel_shader_code( psc_box );
			data_->renderer->set_ps_sampler( "samp", box_sampler );
#else
			data_->renderer->set_pixel_shader(pps_box);
#endif
			data_->renderer->set_blend_shader(pbs_box);
			box_mesh->render();

			data_->renderer->set_rasterizer_state(rs_back);
			data_->renderer->set_vertex_shader(pvs_box);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			data_->renderer->set_pixel_shader_code( psc_box );
			data_->renderer->set_ps_sampler( "samp", box_sampler );
#else
			data_->renderer->set_pixel_shader(pps_box);
#endif
			data_->renderer->set_blend_shader(pbs_box);
			box_mesh->render();
		});
	}

protected:
	mesh_ptr                planar_mesh;
	mesh_ptr                box_mesh;

	texture_ptr             plane_tex;
	texture_ptr             box_tex;

	sampler_ptr             plane_sampler;
	sampler_ptr             box_sampler;

	cpp_vertex_shader_ptr	pvs_box;
	cpp_pixel_shader_ptr	pps_box;
	shader_object_ptr	    psc_box;

	cpp_vertex_shader_ptr	pvs_plane;
	cpp_pixel_shader_ptr	pps_plane;
	shader_object_ptr	    psc_plane;

	cpp_blend_shader_ptr    pbs_box;
	cpp_blend_shader_ptr    pbs_plane;

	raster_state_ptr        rs_front;
	raster_state_ptr        rs_back;
};

EFLIB_MAIN(argc, argv)
{
	texture_and_blending loader;
	loader.init(argc, const_cast<std::_tchar const**>(argv));
	loader.run();

	return 0;
}