#include <salvia/shader/shader.h>
#include <salvia/shader/shader_regs.h>
#include <salvia/shader/shader_object.h>
#include <salvia/core/sync_renderer.h>
#include <salvia/resource/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salvia/common/colors.h>
#include <salvia/resource/texture.h>

#include "salvia/ext/resource/mesh/material.h"
#include "salvia/ext/resource/mesh/mesh_io.h"
#include "salvia/ext/resource/mesh/mesh_io_obj.h"
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

char const* cup_vs_code =
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

char const* mirror_vs_code = 
"float4x4 wvpMatrix; \r\n"
"float4 vs_main(float4 pos: POSITION) :POSITION \r\n"
"{ \r\n"
"	return mul(pos, wvpMatrix); \r\n"
"} \r\n"
;

class cup_ps : public cpp_pixel_shader
{
	salviar::sampler_ptr sampler_;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	int shininess;

public:
	cup_ps()
	{
		declare_constant(_T("Ambient"),   ambient );
		declare_constant(_T("Diffuse"),   diffuse );
		declare_constant(_T("Specular"),  specular );
		declare_constant(_T("Shininess"), shininess );
        declare_sampler(_T("Sampler"), sampler_);
	}

	vec4 to_color(vec3 const& v)
	{
		return vec4(
			(v.x() + 1.0f) / 2.0f,
			(v.y() + 1.0f) / 2.0f,
			(v.z() + 1.0f) / 2.0f,
			1.0f
			);
	}

	bool shader_prog(const vs_output& in, ps_output& out)
	{
		color_rgba32f tex_color(1.0f, 1.0f, 1.0f, 1.0f);
		if( sampler_ )
		{
			tex_color = tex2d(*sampler_ , 0);
		}
		vec3 norm( normalize3( in.attribute(1).xyz() ) );
		vec3 light_dir( normalize3( in.attribute(2).xyz() ) );
		vec3 eye_dir( normalize3( in.attribute(3).xyz() ) );

		float illum_diffuse = clamp( dot_prod3( light_dir, norm ), 0.0f, 1.0f );
		float illum_specular = clamp( dot_prod3( reflect3( light_dir, norm ), eye_dir ), 0.0f, 1.0f );
		vec4 illum = ambient + diffuse * illum_diffuse + specular * illum_specular;

		out.color[0] = tex_color.get_vec4() * illum;
		out.color[0][3] = 1.0f;

		return true;
	}
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}
};

class mirror_ps: public cpp_pixel_shader
{
public:
	mirror_ps()
	{
	}

	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f tex_color(0.3f, 0.3f, 0.1f, 1.0f);
		out.color[0] = tex_color.get_vec4();
		return true;
	}

	virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}
};

class opaque_bs : public cpp_blend_shader
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

class translucent_bs : public cpp_blend_shader
{
public:
	bool shader_prog(size_t sample, pixel_accessor& inout, const ps_output& in)
	{
		// Just disable color writing when drawing mirror.
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

class color_disabled_bs: public cpp_blend_shader
{
	bool shader_prog(size_t /*sample*/, pixel_accessor& /*inout*/, const ps_output& /*in*/)
	{
		// Just disable color writing when drawing mirror.
		return true;
	}

    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}
};

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 3 : 1500;
int const TEST_FRAME_COUNT		= 8;

class stencil_mirror: public sample_app
{
public:
	stencil_mirror(): sample_app("StencilMirror")
	{
	}

protected:
	void on_init() override
	{
		create_devices_and_targets(data_->screen_width, data_->screen_height, 1, pixel_format_color_bgra8, pixel_format_color_rg32f);

		data_->renderer->set_viewport(data_->screen_vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_none;
		rs_back.reset(new raster_state(rs_desc));
		rs_desc.cm = cull_none;
		rs_none.reset(new raster_state(rs_desc));

		depth_stencil_op_desc dsop_desc_replace;
		dsop_desc_replace.stencil_pass_op = stencil_op_replace;

		depth_stencil_op_desc dsop_desc_test;
		dsop_desc_test.stencil_func = compare_function_equal;

		depth_stencil_desc ds_desc, ds_desc_write_stencil, ds_desc_draw_with_stencil;
		dss_normal.reset( new depth_stencil_state(ds_desc) );

		ds_desc_write_stencil.stencil_enable = true;
		ds_desc_write_stencil.depth_func = compare_function_less_equal;
		ds_desc_write_stencil.front_face = dsop_desc_replace;
		ds_desc_write_stencil.back_face = dsop_desc_replace;
		dss_write_stencil.reset( new depth_stencil_state(ds_desc_write_stencil) );

		ds_desc_draw_with_stencil.stencil_enable = true;
		ds_desc_draw_with_stencil.front_face = dsop_desc_test;
		ds_desc_draw_with_stencil.back_face = dsop_desc_test;
		dss_draw_with_stencil.reset( new depth_stencil_state(ds_desc_draw_with_stencil) );

		cup_vs = compile(cup_vs_code, lang_vertex_shader);
		mirror_vs = compile(mirror_vs_code, lang_vertex_shader);
		
		mirror_start_pos = vec3(-3.5f, -1.0f, -3.5f);
		mirror_norm = vec3(0.0f, 1.0f, 0.0f);

		cup_mesh = create_mesh_from_obj( data_->renderer.get(), find_path("cup/cup.obj"), true );
		mirror_mesh = create_planar(
			data_->renderer.get(),
			mirror_norm,
			mirror_start_pos,
			vec3(1.0f, 0.0f, 0.0f),		// major
			vec2(1.0f, 1.0f),			// length
			7, 7, false
			);

		cup_ps_.reset( new cup_ps() );
		mirror_ps_.reset(new mirror_ps());

		opaque_bs_.reset( new opaque_bs() );
		translucent_bs_.reset( new translucent_bs() );
		color_disabled_bs_.reset( new color_disabled_bs() );

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

	void draw_cup(bool reflect)
	{
		if(!cup_vs){ return; }

		data_->renderer->set_rasterizer_state(rs_back);
		data_->renderer->set_vertex_shader_code( cup_vs );
		data_->renderer->set_pixel_shader(cup_ps_);

		vec4 camera_v4 = vec4(camera_pos, 1.0f);
		data_->renderer->set_vs_variable("eyePos", &camera_v4);
		data_->renderer->set_vs_variable("lightPos", &light_pos);

		if(reflect)
		{
			data_->renderer->set_depth_stencil_state(dss_draw_with_stencil, 1);
			data_->renderer->set_blend_shader(translucent_bs_);
			data_->renderer->set_vs_variable("wvpMatrix", &reflect_cup_wvp_matrix);
		}
		else
		{
			data_->renderer->set_depth_stencil_state(dss_normal, 1);	
			data_->renderer->set_blend_shader(opaque_bs_);
			data_->renderer->set_vs_variable("wvpMatrix", &cup_wvp_matrix);
		}

		for( size_t i_mesh = 0; i_mesh < cup_mesh.size(); ++i_mesh )
		{
			mesh_ptr cur_mesh = cup_mesh[i_mesh];

			shared_ptr<obj_material> mtl
				= dynamic_pointer_cast<obj_material>( cur_mesh->get_attached() );
			cup_ps_->set_constant( _T("Ambient"),  &mtl->ambient );
			cup_ps_->set_constant( _T("Diffuse"),  &mtl->diffuse );
			cup_ps_->set_constant( _T("Specular"), &mtl->specular );
			cup_ps_->set_constant( _T("Shininess"),&mtl->ambient );

            sampler_desc desc;
		    desc.min_filter = filter_linear;
		    desc.mag_filter = filter_linear;
		    desc.mip_filter = filter_linear;
		    desc.addr_mode_u = address_clamp;
		    desc.addr_mode_v = address_clamp;
		    desc.addr_mode_w = address_clamp;
                
            if(mtl->tex)
            {
				cup_ps_->set_sampler(_T("Sampler"), data_->renderer->create_sampler(desc, mtl->tex));
            }
            else
            {
                cup_ps_->set_sampler(_T("Sampler"), sampler_ptr());
            }

			cur_mesh->render();
		}
	}

	void draw_mirror()
	{
		data_->renderer->set_depth_stencil_state(dss_write_stencil, 1);
		data_->renderer->set_rasterizer_state(rs_none);
		data_->renderer->set_vertex_shader_code(mirror_vs);
		data_->renderer->set_pixel_shader(mirror_ps_);
		data_->renderer->set_vs_variable("wvpMatrix", &mirror_wvp_matrix); 
		data_->renderer->set_blend_shader(opaque_bs_);
		
		mirror_mesh->render();
	}

	void on_frame() override
	{
		profiling("BackBufferClearing", [this](){
			profiling("ClearingDS", [this](){
				data_->renderer->clear_color(data_->color_target, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
				data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth | clear_stencil, 1.0f, 0);
			});
		});

		float scene_sec = 0.0f;
		switch(data_->mode)
		{
		case app_modes::benchmark:
			scene_sec = static_cast<float>(data_->frame_count) / (BENCHMARK_FRAME_COUNT - 1);
			break;
		case app_modes::test:
			scene_sec = static_cast<float>(data_->frame_count) / (TEST_FRAME_COUNT - 1);
			break;
		default:
			scene_sec = static_cast<float>(data_->total_elapsed_sec);
			break;
		}

		float angle0 = 60.0f;
		float angle1 = scene_sec * 60.0f * (static_cast<float>(TWO_PI) / 360.0f) * 0.25f;

		camera_pos = vec3(cos(angle0)*5.0f, 2.5f, sin(angle0)*5.0f);
		
		mat44 world(mat44::identity()), world_reflect, view, proj, viewproj, mirror_mat;

		mat_translate(world, 0.0f, 0.0f, 0.0f);
		mat_lookat(view, camera_pos, vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), data_->screen_aspect_ratio, 0.1f, 100.0f);
		mat_mul(viewproj, view, proj);
		mat_mul(mirror_wvp_matrix, world, viewproj);

		mat_translate(world, cos(angle1)*3.0f, 0.0f, sin(angle1)*3.0f);
		mat_mul(cup_wvp_matrix, world, viewproj);

		vec4 mirror_plane(mirror_norm, -dot_prod3(mirror_norm, mirror_start_pos));
		mat_reflect(mirror_mat, mirror_plane);
		mat_mul(world_reflect, world, mirror_mat);
		mat_mul(reflect_cup_wvp_matrix, world_reflect, viewproj);

		light_pos = vec4(sin(-angle0*1.5f)*2.2f, 0.15f, cos(angle0*0.9f)*1.8f, 0.0f);

		profiling("Rendering", [this](){
			profiling("Scene",  [&]{ draw_cup(false); });
			profiling("Mirror", [&]{ draw_mirror();   });
		});

		profiling("BackBufferClearing", [this](){
			profiling("ClearingDepth", [this](){
				data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth, 1.0f, 0);
			});
		});

		profiling("Rendering", [this](){
			profiling("InMirror",  [&]{ draw_cup(true); });
		});		
	}

protected:
	vector<mesh_ptr>        cup_mesh;
	mesh_ptr				mirror_mesh;

	vec3					mirror_start_pos;
	vec3					mirror_norm;

	vec4					light_pos;
	vec3					camera_pos;
	mat44					mirror_wvp_matrix;
	mat44					cup_wvp_matrix;
	mat44					reflect_cup_wvp_matrix;

	shader_object_ptr       mirror_vs;
	shader_object_ptr       cup_vs;

	cpp_pixel_shader_ptr    cup_ps_;
	cpp_pixel_shader_ptr    mirror_ps_;

	cpp_blend_shader_ptr    opaque_bs_;
	cpp_blend_shader_ptr    translucent_bs_;
	cpp_blend_shader_ptr    color_disabled_bs_;

	raster_state_ptr        rs_back;
	raster_state_ptr		rs_none;

	depth_stencil_state_ptr	dss_normal;
	depth_stencil_state_ptr	dss_write_stencil;
	depth_stencil_state_ptr	dss_draw_with_stencil;
};

EFLIB_MAIN(argc, argv)
{
	stencil_mirror loader;
	loader.init(argc, const_cast<std::_tchar const**>(argv));
	loader.run();

	return 0;
}
