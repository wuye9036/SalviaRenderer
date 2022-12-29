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

#include <eflib/platform/dl_loader.h>
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

static float const esm_constant = 25000.0f;
static float const gaussian_weights[] =
{
	0.027681f, 0.111014f, 0.027681f,
	0.111014f, 0.445213f, 0.111014f,
	0.027681f, 0.111014f, 0.027681f
};

class gen_sm_cpp_ps : public cpp_pixel_shader
{
    bool output_depth() const
    {
        return true;
    }

	bool shader_prog(const vs_output& in, ps_output& out)
	{
		EFLIB_UNREF_DECLARATOR(in);
		EFLIB_UNREF_DECLARATOR(out);
		return true;
	}

    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}
};

class draw_cpp_ps : public cpp_pixel_shader
{
	sampler_ptr	texsamp_;
	sampler_ptr dsamp_;

	vec4		ambient;
	vec4		diffuse;
	vec4		specular;
	int			shininess;

public:
	draw_cpp_ps()
	{
		declare_constant(_T("Ambient"),     ambient );
		declare_constant(_T("Diffuse"),     diffuse );
		declare_constant(_T("Specular"),    specular );
		declare_constant(_T("Shininess"),   shininess );
        declare_sampler (_T("DepthSampler"),dsamp_);
        declare_sampler (_T("TexSampler"),  texsamp_);
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
        float occlusion = 0.0f;
        if(dsamp_)
        {
            vec3 lis_pos( in.attribute(4).xyz() / in.attribute(4).w() );
            vec2 sm_center( (lis_pos.x() + 1.0f)*0.5f, (1.0f-(lis_pos.y()+1.0f)*0.5f) );

			float sm_offset = 1 / 512.0f;
			vec2 sm_coords[] = 
			{
				sm_center + vec2(-sm_offset, -sm_offset),
				sm_center + vec2(      0.0f, -sm_offset),
				sm_center + vec2(+sm_offset, -sm_offset),
				sm_center + vec2(-sm_offset,       0.0f),
				sm_center + vec2(      0.0f,       0.0f),
				sm_center + vec2(+sm_offset,       0.0f),
				sm_center + vec2(-sm_offset, +sm_offset),
				sm_center + vec2(      0.0f, +sm_offset),
				sm_center + vec2(+sm_offset, +sm_offset)
			};

            float shadow_depth[9];
			for(int i = 0; i < 9; ++i)
			{
				vec4 sm_coord_lod(sm_coords[i].x(), sm_coords[i].y(), 0.0f, 0.0f);
				shadow_depth[i] = tex2dlod(*dsamp_, sm_coord_lod).r;
			}

			float occluder = 0.0f;
			for(int i = 1; i < 9; ++i)
			{
				occluder += gaussian_weights[i] * exp(esm_constant * (shadow_depth[i] - shadow_depth[0]));
			}
			occluder += gaussian_weights[0];
			occluder = log(occluder);
			occluder += esm_constant * shadow_depth[0];

			occlusion = eflib::clamp(exp(occluder - esm_constant * lis_pos[2]), 0.0f, 1.0f);
        }

        color_rgba32f tex_color(1.0f, 1.0f, 1.0f, 1.0f);
        if(texsamp_)
        {
            tex_color = tex2d(*texsamp_ , 0);
        }

		vec3 norm( normalize3( in.attribute(1).xyz() ) );
		vec3 light_dir( normalize3( in.attribute(2).xyz() ) );
		vec3 eye_dir( normalize3( in.attribute(3).xyz() ) );
		
		float illum_diffuse = clamp( dot_prod3( light_dir, norm ), 0.0f, 1.0f );
		float illum_specular = clamp( dot_prod3(-reflect3(light_dir, norm), eye_dir), 0.0f, 1.0f );
		vec4 illum = ambient + ( diffuse * illum_diffuse + specular * pow(illum_specular, shininess) ) * occlusion;

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

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 3 : 500;
int const TEST_FRAME_COUNT		= 8;

class ssm: public sample_app
{
public:
	ssm(): sample_app("StandardShadowMap")
	{}

protected:
	void on_init() override
	{	
		create_devices_and_targets(data_->screen_width, data_->screen_height, 1, pixel_format_color_bgra8, pixel_format_color_rg32f);

		data_->renderer->set_viewport(data_->screen_vp);

        sm_texture_ = data_->renderer->create_tex2d(data_->screen_width, data_->screen_height, 1, pixel_format_color_rg32f);

		sampler_desc sm_desc;
		sm_desc.min_filter = filter_point;
		sm_desc.mag_filter = filter_point;
		sm_desc.mip_filter = filter_point;
		sm_desc.addr_mode_u = address_border;
		sm_desc.addr_mode_v = address_border;
		sm_desc.addr_mode_w = address_border;
		sm_desc.border_color = color_rgba32f(1.0f, 0.0f, 0.0f, 0.0f);
        sm_sampler_ = data_->renderer->create_sampler(sm_desc, sm_texture_);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

		gen_sm_vs_ = compile_from_file(find_path("ssm/GenSM.savs"), lang_vertex_shader);
		draw_vs_ = compile_from_file(find_path("ssm/Draw.savs"), lang_vertex_shader);
		gen_sm_ps_.reset(new gen_sm_cpp_ps());
		draw_ps_.reset(new draw_cpp_ps());
		pbs.reset( new bs() );
		
		cup_mesh = create_mesh_from_obj( data_->renderer.get(), find_path("cup/cup.obj"), true );
        plane_mesh = create_planar(
            data_->renderer.get(),
            vec3(-3.0f, 0.0f, -3.0f),
            vec3(6.0f, -1.0f, 0.0f),
            vec3(0.0f, -1.0f, 6.0f),
            1, 1, false
            );

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

	void gen_sm()
	{
		data_->renderer->set_render_targets(0, nullptr, sm_texture_->subresource(0));
		
		profiling("BackBufferClearing", [&](){
			profiling("Shadow", [&](){
				data_->renderer->clear_depth_stencil(sm_texture_->subresource(0), clear_depth | clear_stencil, 1.0f, 0);
			});
		});

        data_->renderer->set_vertex_shader_code(gen_sm_vs_);
		data_->renderer->set_pixel_shader(gen_sm_ps_);
		data_->renderer->set_blend_shader(pbs);

		data_->renderer->set_rasterizer_state(rs_back);

        data_->renderer->set_vs_variable("wvpMatrix",	&light_wvp_);
		
		profiling("Rendering", [&](){
			profiling("Shadow", [&](){
				plane_mesh->render();
				for( size_t i_mesh = 0; i_mesh < cup_mesh.size(); ++i_mesh )
				{
					mesh_ptr cur_mesh = cup_mesh[i_mesh];
					cur_mesh->render();
				}
			});
		});
	}
	
	void draw()
	{
		data_->renderer->set_render_targets(1, &data_->color_target, data_->ds_target);
		profiling("BackBufferClearing", [&](){
			profiling("Scene", [&](){
				data_->renderer->clear_color(data_->color_target, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
				data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth | clear_stencil, 1.0f, 0);
			});
		});
		data_->renderer->set_vertex_shader_code(draw_vs_);
		data_->renderer->set_pixel_shader(draw_ps_);
		data_->renderer->set_blend_shader(pbs);

		data_->renderer->set_rasterizer_state(rs_back);

		data_->renderer->set_vs_variable("cameraPos",	&camera_pos_);
		data_->renderer->set_vs_variable("cameraWvp", &camera_wvp_);
		data_->renderer->set_vs_variable("lightPos",	&light_pos_);
		data_->renderer->set_vs_variable("lightWvp",	&light_wvp_);

        vec4 ambient (0.1f, 0.1f, 0.1f, 0.1f);
        vec4 diffuse (0.8f, 0.8f, 0.8f, 0.1f);
        vec4 specular(0.4f, 0.4f, 0.4f, 0.1f);
        int  shininess(32);
        draw_ps_->set_constant( _T("Ambient"),  &ambient );
	    draw_ps_->set_constant( _T("Diffuse"),  &diffuse );
		draw_ps_->set_constant( _T("Specular"), &specular );
        draw_ps_->set_constant( _T("Shininess"),&shininess);
        draw_ps_->set_sampler(_T("TexSampler"), sampler_ptr());
        draw_ps_->set_sampler(_T("DepthSampler"), sm_sampler_);

	    sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_linear;
		desc.addr_mode_u = address_clamp;
		desc.addr_mode_v = address_clamp;
		desc.addr_mode_w = address_clamp;

		profiling("Rendering", [&](){
			profiling("Scene", [&](){
				plane_mesh->render();
				for (size_t i_mesh = 0; i_mesh < cup_mesh.size(); ++i_mesh)
				{
					mesh_ptr cur_mesh = cup_mesh[i_mesh];

					shared_ptr<obj_material> mtl
						= dynamic_pointer_cast<obj_material>(cur_mesh->get_attached());
					draw_ps_->set_constant(_T("Ambient"), &mtl->ambient);
					draw_ps_->set_constant(_T("Diffuse"), &mtl->diffuse);
					draw_ps_->set_constant(_T("Specular"), &mtl->specular);
					draw_ps_->set_constant(_T("Shininess"), &mtl->ambient);

					if (mtl->tex)
					{
						draw_ps_->set_sampler(_T("TexSampler"), data_->renderer->create_sampler(desc, mtl->tex));
					}
					else
					{
						draw_ps_->set_sampler(_T("TexSampler"), sampler_ptr());
					}
					draw_ps_->set_sampler(_T("DepthSampler"), sm_sampler_);

					cur_mesh->render();
				}
			});
		});
	}
	
	void on_frame() override
    {
		if(!gen_sm_vs_ || !draw_vs_ || !gen_sm_ps_ || !draw_ps_)
		{
			return;
		}

		camera_pos_ = vec4(6.0f, 3.1f, 3.0f, 1.0f);
		mat44 view, proj;
		mat_lookat(view, camera_pos_.xyz(), vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI * 0.5f), data_->screen_aspect_ratio, 0.1f, 100.0f);
		mat_mul(camera_wvp_, view, proj);

		float scene_sec = 0.0f;
		switch(data_->mode)
		{
		case app_modes::benchmark:
			scene_sec = static_cast<float>(data_->frame_count * 3.3f) / (BENCHMARK_FRAME_COUNT - 1);
			break;
		case app_modes::test:
			scene_sec = static_cast<float>(data_->frame_count * 3.3f) / (TEST_FRAME_COUNT - 1);
			break;
		default:
			scene_sec = static_cast<float>(data_->total_elapsed_sec);
			break;
		}

		float theta = 0.3f * scene_sec;
		light_pos_ = vec4(-4.0f * sin(theta), 6.1f, 3.5f * cos(theta), 1.0f);
		mat_lookat(view, light_pos_.xyz(), vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI * 0.5f), data_->screen_aspect_ratio, 0.1f, 40.0f);
		mat_mul(light_wvp_, view, proj);

		gen_sm();
		draw();
	}

protected:
    texture_ptr             sm_texture_;
    sampler_ptr             sm_sampler_;

	vec4					camera_pos_;
	mat44					camera_wvp_;
	vec4					light_pos_;
	mat44					light_wvp_;
	
	vector<mesh_ptr>        cup_mesh;
    mesh_ptr                plane_mesh;

	shader_object_ptr		gen_sm_vs_;
	cpp_pixel_shader_ptr    gen_sm_ps_;
	shader_object_ptr		draw_vs_;
	cpp_pixel_shader_ptr    draw_ps_;
	
	cpp_blend_shader_ptr    pbs;

	raster_state_ptr        rs_back;
};

EFLIB_MAIN(argc, argv)
{
	ssm loader;
	loader.init(argc, const_cast<std::_tchar const**>(argv));
	loader.run();

	return 0;
}