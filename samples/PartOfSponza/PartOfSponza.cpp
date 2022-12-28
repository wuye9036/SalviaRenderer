#include "salvia/ext/resource/mesh/material.h"
#include "salvia/ext/resource/mesh/mesh_io.h"
#include "salvia/ext/resource/mesh/mesh_io_obj.h"
#include <salvia/ext/swap_chain/swap_chain.h>

#include <salvia/shader/shader.h>
#include <salvia/shader/shader_regs.h>
#include <salvia/shader/shader_object.h>
#include <salvia/core/sync_renderer.h>
#include <salvia/resource/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salvia/common/colors.h>
#include <salvia/resource/texture.h>

#include <salviau/include/common/sample_app.h>
#include <salviau/include/common/path.h>

#include <eflib/platform/main.h>

#include <vector>

using namespace eflib;
using namespace salvia::core;
using namespace salviax;
using namespace salvia::ext::resource;
using namespace salviau;

using std::shared_ptr;
using std::dynamic_pointer_cast;

using std::string;
using std::vector;
using std::cout;
using std::endl;

#define SASL_VERTEX_SHADER_ENABLED

char const* sponza_vs_code =
"float4x4 wvpMatrix; \r\n"
"float4   eyePos; \r\n"
"float4	  lightPos; \r\n"
"struct VSIn{ \r\n"
"	float4 pos: POSITION; \r\n"
"	float4 norm: NORMAL; \r\n"
"	float4 tex: TEXCOORD0; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: sv_position; \r\n"
"	float4 tex: TEXCOORD; \r\n"
"	float4 norm: TEXCOORD1; \r\n"
"	float4 lightDir: TEXCOORD2; \r\n"
"	float4 eyeDir: TEXCOORD3; \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
"	out.norm = in.norm; \r\n"
"	out.pos = mul(in.pos, wvpMatrix); \r\n"
"	out.lightDir = lightPos - in.pos; \r\n"
"	out.eyeDir = eyePos - in.pos; \r\n"
"	out.tex = in.tex; \r\n"
"	return out; \r\n"
"} \r\n"
;

class sponza_vs : public cpp_vertex_shader
{
	mat44 wvp;
	vec4 light_pos, eye_pos;
public:
	sponza_vs():wvp(mat44::identity()){
		declare_constant(_T("wvpMatrix"), wvp);
		declare_constant(_T("lightPos"), light_pos);
		declare_constant(_T("eyePos"), eye_pos );

		bind_semantic( "POSITION", 0, 0 );
		bind_semantic( "TEXCOORD", 0, 1 );
		bind_semantic( "NORMAL", 0, 2 );
	}

	sponza_vs(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attribute(0);
		transform(out.position(), pos, wvp);
		out.attribute(0) = in.attribute(1);
		out.attribute(1) = in.attribute(2);
		out.attribute(2) = light_pos - pos;
		out.attribute(3) = eye_pos - pos;
	}

	uint32_t num_output_attributes() const{
		return 4;
	}

	uint32_t output_attribute_modifiers(uint32_t) const{
		return salviar::vs_output::am_linear;
	}

    virtual cpp_shader_ptr clone()
    {
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
    }
};


class sponza_ps : public cpp_pixel_shader
{
	salviar::sampler_ptr sampler_;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	int shininess;

public:
	sponza_ps()
	{
		declare_constant(_T("Ambient"),   ambient );
		declare_constant(_T("Diffuse"),   diffuse );
		declare_constant(_T("Specular"),  specular );
		declare_constant(_T("Shininess"), shininess );
        declare_sampler(_T("Sampler"), sampler_);
	}


	bool shader_prog(const vs_output& in, ps_output& out)
	{
		vec4 diff_color = vec4(1.0f, 1.0f, 1.0f, 1.0f); // diffuse;

		if( sampler_ )
		{
			diff_color = tex2d(*sampler_, 0).get_vec4();
		}

		vec3 norm( normalize3( in.attribute(1).xyz() ) );
		vec3 light_dir( normalize3( in.attribute(2).xyz() ) );
		vec3 eye_dir( normalize3( in.attribute(3).xyz() ) );

		float illum_diffuse = clamp( dot_prod3( light_dir, norm ), 0.0f, 1.0f );
		float illum_specular = clamp( dot_prod3( reflect3( light_dir, norm ), eye_dir ), 0.0f, 1.0f );

		out.color[0] = ambient * 0.01f + diff_color * illum_diffuse + specular * illum_specular;
		out.color[0] = diff_color * illum_diffuse;
		out.color[0][3] = 1.0f;
		
		return true;
	}
	virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}
};


class bs : public cpp_blend_shader
{
public:
	bool shader_prog(size_t sample, pixel_accessor& inout, const ps_output& in)
	{
		color_rgba32f color(in.color[0]);
		inout.color( 0, sample, color_rgba32f(in.color[0]) );
		return true;
	}

    virtual cpp_shader_ptr clone()
    {
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
    }
};

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 3 : 600;
int const TEST_FRAME_COUNT		= 8;

class sponza: public sample_app
{
public:
	sponza(): sample_app("PartOfSponza") {}

protected:
	void on_init() override
    {
		create_devices_and_targets(data_->screen_width, data_->screen_height, 1, pixel_format_color_bgra8, pixel_format_color_rg32f);
		data_->renderer->set_viewport(data_->screen_vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

#ifdef SASL_VERTEX_SHADER_ENABLED
		cout << "Compiling vertex shader ... " << endl;
		sponza_sc = compile( sponza_vs_code, lang_vertex_shader );
#endif

		cout << "Loading mesh ... " << endl;
#ifdef EFLIB_DEBUG
		cout << "Application is built in debug mode. Mesh loading is *VERY SLOW*." << endl;
#endif
		sponza_mesh = create_mesh_from_obj( data_->renderer.get(), find_path("sponza_lq/part_of_sponza.obj"), false );
		cout << "Loading pixel and blend shader... " << endl;

		pvs.reset( new sponza_vs() );
		pps.reset( new sponza_ps() );
		pbs.reset( new bs() );

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

		float xpos = -36.0f + fmodf(10.0f * scene_sec, 66.0f);

		vec3 camera( xpos, 8.0f, 0.0f);
		vec4 camera_pos = vec4( camera, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;
		mat_lookat(view, camera, vec3(40.0f, 15.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), data_->screen_aspect_ratio, 0.1f, 1000.0f);

		float ypos = 10.0f + fmodf(8.0f * scene_sec, 40.0f);
		vec4 lightPos( 0.0f, ypos, 0.0f, 1.0f );

		data_->renderer->set_pixel_shader(pps);
		data_->renderer->set_blend_shader(pbs);

		mat_translate(world , -0.5f, 0, -0.5f);
		mat_mul(wvp, world, mat_mul(wvp, view, proj));

		data_->renderer->set_rasterizer_state(rs_back);

		// C++ vertex shader and SASL vertex shader are all available.
#ifdef SASL_VERTEX_SHADER_ENABLED
		data_->renderer->set_vertex_shader_code( sponza_sc );
#else
		pvs->set_constant( _T("wvpMatrix"), &wvp );
		pvs->set_constant( _T("eyePos"), &camera_pos );
		pvs->set_constant( _T("lightPos"), &lightPos );
		data_->renderer->set_vertex_shader(pvs);
#endif
		data_->renderer->set_vs_variable( "wvpMatrix", &wvp );
			
		data_->renderer->set_vs_variable( "eyePos", &camera_pos );
		data_->renderer->set_vs_variable( "lightPos", &lightPos );

		profiling("Rendering", [&](){
			for( size_t i_mesh = 0; i_mesh < sponza_mesh.size(); ++i_mesh )
			{
				mesh_ptr cur_mesh = sponza_mesh[i_mesh];

				shared_ptr<obj_material> mtl
					= dynamic_pointer_cast<obj_material>( cur_mesh->get_attached() );

				pps->set_constant( _T("Ambient"),  &mtl->ambient );
				pps->set_constant( _T("Diffuse"),  &mtl->diffuse );
				pps->set_constant( _T("Specular"), &mtl->specular );
				pps->set_constant( _T("Shininess"),&mtl->ambient );

				sampler_desc desc;
				desc.min_filter = filter_linear;
				desc.mag_filter = filter_linear;
				desc.mip_filter = filter_linear;
				desc.addr_mode_u = address_wrap;
				desc.addr_mode_v = address_wrap;
				desc.addr_mode_w = address_wrap;
				pps->set_sampler(_T("Sampler"), data_->renderer->create_sampler(desc, mtl->tex));

				cur_mesh->render();
			}
		});
	}

protected:
	vector<mesh_ptr>            sponza_mesh;
	shared_ptr<shader_object>   sponza_sc;

	cpp_vertex_shader_ptr		pvs;
	cpp_pixel_shader_ptr		pps;
	cpp_blend_shader_ptr		pbs;

	raster_state_ptr			rs_back;
};

EFLIB_MAIN(argc, argv)
{
	sponza loader;
	loader.init(argc, const_cast<std::_tchar const**>(argv));
	loader.run();

	return 0;
}