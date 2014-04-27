#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_regs.h>
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

class stencil_mirror: public quick_app
{
public:
	stencil_mirror(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create()
	{
		string title( "Sample: Stencil Mirror" );
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
        ds_surface_ = renderer_->create_tex2d(
            render_params.backbuffer_width,
            render_params.backbuffer_height,
            render_params.backbuffer_num_samples,
            pixel_format_color_rg32f
            )->get_surface(0);
        renderer_->set_render_targets(1, &color_surface_, ds_surface_);
        
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

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		mirror_start_pos = vec3(-2.0f, -1.0f, -2.0f);
		mirror_norm = vec3(0.0f, 1.0f, 0.0f);

		cup_mesh = create_mesh_from_obj( renderer_.get(), "../../resources/models/cup/cup.obj", true );
		mirror_mesh = create_planar(
			renderer_.get(),
			mirror_norm,
			mirror_start_pos,
			vec3(1.0f, 0.0f, 0.0f),		// major
			vec2(1.0f, 1.0f),			// length
			4, 4, false
			);

		cup_ps_.reset( new cup_ps() );
		mirror_ps_.reset(new mirror_ps());

		opaque_bs_.reset( new opaque_bs() );
		translucent_bs_.reset( new translucent_bs() );
		color_disabled_bs_.reset( new color_disabled_bs() );
	}
	/** @} */

	void on_draw()
    {
		swap_chain_->present();
	}

	void draw_cup(bool reflect)
	{
		if(!cup_vs){ return; }

		renderer_->set_rasterizer_state(rs_back);
		renderer_->set_vertex_shader_code( cup_vs );
		renderer_->set_pixel_shader(cup_ps_);

		vec4 camera_v4 = vec4(camera_pos, 1.0f);
		renderer_->set_vs_variable("eyePos", &camera_v4);
		renderer_->set_vs_variable("lightPos", &light_pos);

		if(reflect)
		{
			renderer_->set_depth_stencil_state(dss_draw_with_stencil, 1);
			renderer_->set_blend_shader(translucent_bs_);
			renderer_->set_vs_variable("wvpMatrix", &reflect_cup_wvp_matrix);
		}
		else
		{
			renderer_->set_depth_stencil_state(dss_normal, 1);	
			renderer_->set_blend_shader(opaque_bs_);
			renderer_->set_vs_variable("wvpMatrix", &cup_wvp_matrix);
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
				cup_ps_->set_sampler(_T("Sampler"), renderer_->create_sampler(desc, mtl->tex));
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
		renderer_->set_depth_stencil_state(dss_write_stencil, 1);
		renderer_->set_rasterizer_state(rs_none);
		renderer_->set_vertex_shader_code(mirror_vs);
		renderer_->set_pixel_shader(mirror_ps_);
		renderer_->set_vs_variable("wvpMatrix", &mirror_wvp_matrix); 
		renderer_->set_blend_shader(opaque_bs_);
		
		mirror_mesh->render();
	}

	void on_idle()
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

		static float angle0 = 60.0f;
		static float angle1 = 0;
		// angle0 -= elapsed_time * 60.0f * (static_cast<float>(TWO_PI) / 360.0f) * 0.15f;
		angle1 += elapsed_time * 60.0f * (static_cast<float>(TWO_PI) / 360.0f) * 0.25f;

		camera_pos = vec3(cos(angle0)*5.0f, 2.5f, sin(angle0)*5.0f);
		
		mat44 world(mat44::identity()), world_reflect, view, proj, viewproj, mirror_mat;

		mat_translate(world, 0.0f, 0.0f, 0.0f);
		mat_lookat(view, camera_pos, vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 100.0f);
		mat_mul(viewproj, view, proj);
		mat_mul(mirror_wvp_matrix, world, viewproj);

		mat_translate(world, cos(angle1)*3.0f, 0.0f, sin(angle1)*3.0f);
		mat_mul(cup_wvp_matrix, world, viewproj);

		vec4 mirror_plane(mirror_norm, -dot_prod3(mirror_norm, mirror_start_pos));
		mat_reflect(mirror_mat, mirror_plane);
		mat_mul(world_reflect, world, mirror_mat);
		mat_mul(reflect_cup_wvp_matrix, world_reflect, viewproj);

		light_pos = vec4(sin(-angle0*1.5f)*2.2f, 0.15f, cos(angle0*0.9f)*1.8f, 0.0f);

        renderer_->clear_color(color_surface_, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		renderer_->clear_depth_stencil(ds_surface_, clear_depth | clear_stencil, 1.0f, 0);
		
		draw_cup(false);
		draw_mirror();

		renderer_->clear_depth_stencil(ds_surface_, clear_depth, 1.0f, 0);
		draw_cup(true);

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	swap_chain_ptr          swap_chain_;
	renderer_ptr            renderer_;
    surface_ptr             ds_surface_;
    surface_ptr             color_surface_;

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

	uint32_t                num_frames;
	float                   accumulate_time;
	float                   fps;

	timer                   timer;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] )
{
	stencil_mirror app;
	return app.run();
}
