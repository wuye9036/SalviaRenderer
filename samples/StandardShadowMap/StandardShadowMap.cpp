#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/shader.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>
#include <salviar/include/texture.h>

#include <salviax/include/swap_chain/swap_chain.h>
#include <salviax/include/resource/mesh/sa/material.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>

#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#include <vector>

using namespace eflib;
using namespace salviar;
using namespace salviax;
using namespace salviax::resource;
using namespace salviau;

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

using std::string;
using std::vector;
using std::cout;
using std::endl;

static float const esm_constant = 4000.0f;

class gen_sm_cpp_ps : public cpp_pixel_shader
{
    bool output_depth() const
    {
        return true;
    }

	bool shader_prog(const vs_output& in, ps_output& out)
	{
        out.depth = in.position().z();
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
            vec3 light_space_pos( in.attribute(4).xyz() / in.attribute(4).w() );
            vec3 light_texture_coord = (light_space_pos + vec3(1.0f, 1.0f, 1.0f)) * 0.5f;
            light_texture_coord.y( 1.0f - light_texture_coord.y() );
            float shadow_depth = tex2dlod(*dsamp_, vec4(light_texture_coord.xyz(), 0.0f)).r;
            occlusion = eflib::clamp(exp(esm_constant * (shadow_depth - light_space_pos.z())), 0.0f, 1.0f);
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

class ssm: public quick_app
{
public:
	ssm()
		: quick_app( create_wtl_application() )
		, fps_counter_(3.0f)
	{}

protected:
	/** Event handlers @{ */
	virtual void on_create()
	{
		string title( "Sample: Obj File Loader" );
		impl->main_window()->set_title( title );
		boost::any view_handle_any = impl->main_window()->view_handle();
        void* window_handle = *boost::unsafe_any_cast<void*>(&view_handle_any);
		
		renderer_parameters render_params = {0};
		render_params.backbuffer_format = pixel_format_color_bgra8;
		render_params.backbuffer_height = 512;
		render_params.backbuffer_width = 512;
		render_params.backbuffer_num_samples = 1;
        render_params.native_window = window_handle;

        salviax_create_swap_chain_and_renderer(swap_chain_, renderer_, &render_params, renderer_sync);
        color_surface_ = swap_chain_->get_surface();
        sm_texture_ = renderer_->create_tex2d(
            render_params.backbuffer_width,
            render_params.backbuffer_height,
            render_params.backbuffer_num_samples,
            pixel_format_color_rg32f
            );
		draw_ds_surface_ = renderer_->create_tex2d(
            render_params.backbuffer_width,
            render_params.backbuffer_height,
            render_params.backbuffer_num_samples,
            pixel_format_color_rg32f
            )->get_surface(0);

		sampler_desc sm_desc;
		sm_desc.min_filter = filter_point;
		sm_desc.mag_filter = filter_point;
		sm_desc.mip_filter = filter_point;
		sm_desc.addr_mode_u = address_border;
		sm_desc.addr_mode_v = address_border;
		sm_desc.addr_mode_w = address_border;
		sm_desc.border_color = color_rgba32f(1.0f, 0.0f, 0.0f, 0.0f);
        sm_sampler_ = renderer_->create_sampler(sm_desc, sm_texture_);

        viewport vp;
        vp.w = static_cast<float>(render_params.backbuffer_width);
        vp.h = static_cast<float>(render_params.backbuffer_height);
        vp.x = 0;
        vp.y = 0;
        vp.minz = 0.0f;
        vp.maxz = 1.0f;
        renderer_->set_viewport(vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

		gen_sm_vs_ = compile_from_file("../../resources/shaders/ssm/GenSM.savs", lang_vertex_shader);
		draw_vs_ = compile_from_file("../../resources/shaders/ssm/Draw.savs", lang_vertex_shader);
		gen_sm_ps_.reset(new gen_sm_cpp_ps());
		draw_ps_.reset(new draw_cpp_ps());
		pbs.reset( new bs() );
		
		cup_mesh = create_mesh_from_obj( renderer_.get(), "../../resources/models/cup/cup.obj", true );
        plane_mesh = create_planar(
            renderer_.get(),
            vec3(-3.0f, 0.0f, -3.0f),
            vec3(6.0f, -1.0f, 0.0f),
            vec3(0.0f, -1.0f, 6.0f),
            1, 1, false
            );
	}
	/** @} */

	void on_draw()
    {
		swap_chain_->present();
	}

	void gen_sm()
	{
		renderer_->set_render_targets(0, nullptr, sm_texture_->get_surface(0));
        renderer_->clear_depth_stencil(sm_texture_->get_surface(0), 1.0f, 0);

        renderer_->set_vertex_shader_code(gen_sm_vs_);
		renderer_->set_pixel_shader(gen_sm_ps_);
		renderer_->set_blend_shader(pbs);

		renderer_->set_rasterizer_state(rs_back);

        renderer_->set_vs_variable("wvpMatrix",	&light_wvp_);
        plane_mesh->render();

        for( size_t i_mesh = 0; i_mesh < cup_mesh.size(); ++i_mesh )
		{
			mesh_ptr cur_mesh = cup_mesh[i_mesh];
			cur_mesh->render();
		}
	}
	
	void draw()
	{
		renderer_->set_render_targets(1, &color_surface_, draw_ds_surface_);

		renderer_->clear_color(color_surface_, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		renderer_->clear_depth_stencil(draw_ds_surface_, 1.0f, 0);

		renderer_->set_vertex_shader_code(draw_vs_);
		renderer_->set_pixel_shader(draw_ps_);
		renderer_->set_blend_shader(pbs);

		renderer_->set_rasterizer_state(rs_back);

		renderer_->set_vs_variable("cameraPos",	&camera_pos_);
		renderer_->set_vs_variable("cameraWvp", &camera_wvp_);
		renderer_->set_vs_variable("lightPos",	&light_pos_);
		renderer_->set_vs_variable("lightWvp",	&light_wvp_);

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

        plane_mesh->render();
		for( size_t i_mesh = 0; i_mesh < cup_mesh.size(); ++i_mesh )
		{
			mesh_ptr cur_mesh = cup_mesh[i_mesh];

			shared_ptr<obj_material> mtl
				= dynamic_pointer_cast<obj_material>( cur_mesh->get_attached() );
			draw_ps_->set_constant( _T("Ambient"),  &mtl->ambient );
			draw_ps_->set_constant( _T("Diffuse"),  &mtl->diffuse );
			draw_ps_->set_constant( _T("Specular"), &mtl->specular );
			draw_ps_->set_constant( _T("Shininess"),&mtl->ambient );

        	sampler_desc desc;
		    desc.min_filter = filter_linear;
		    desc.mag_filter = filter_linear;
		    desc.mip_filter = filter_linear;
		    desc.addr_mode_u = address_clamp;
		    desc.addr_mode_v = address_clamp;
		    desc.addr_mode_w = address_clamp;

            if(mtl->tex)
            {
                draw_ps_->set_sampler(_T("TexSampler"), renderer_->create_sampler(desc, mtl->tex));
            }
            else
            {
                draw_ps_->set_sampler(_T("TexSampler"), sampler_ptr());
            }
            draw_ps_->set_sampler(_T("DepthSampler"), sm_sampler_);

			cur_mesh->render();
		}
	}
	
	void on_idle()
    {
		float fps;
		if( fps_counter_.on_frame(fps) )
		{
			cout << fps << endl;
		}

		if(!gen_sm_vs_ || !draw_vs_ || !gen_sm_ps_ || !draw_ps_)
		{
			return;
		}

		camera_pos_ = vec4(6.0f, 3.1f, 3.0f, 1.0f);
		mat44 view, proj;
		mat_lookat(view, camera_pos_.xyz(), vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI * 0.5f), 1.0f, 0.1f, 100.0f);
		mat_mul(camera_wvp_, view, proj);

        static float theta = 0.0f;
        theta += 0.01f;
		light_pos_ = vec4(-4.0f * sin(theta), 6.1f, 3.5f * cos(theta), 1.0f);
		mat_lookat(view, light_pos_.xyz(), vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI * 0.5f), 1.0f, 0.1f, 40.0f);
		mat_mul(light_wvp_, view, proj);

		gen_sm();
		draw();
		
		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	swap_chain_ptr          swap_chain_;
	renderer_ptr            renderer_;

    texture_ptr             sm_texture_;
	surface_ptr				draw_ds_surface_;
    surface_ptr             color_surface_;
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

	fps_counter				fps_counter_;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] )
{
	ssm loader;
	return loader.run();
}