#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/shader.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

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

class cpp_draw_ps : public cpp_pixel_shader
{
	salviar::sampler_ptr sampler_;
	salviar::texture_ptr tex_;

	vec4	ambient;
	vec4	diffuse;
	vec4	specular;
	int		shininess;

public:
	void set_texture( salviar::texture_ptr tex )
	{
		tex_ = tex;
		sampler_->set_texture(tex_.get());
	}

	cpp_draw_ps()
	{
		declare_constant(_T("Ambient"),   ambient );
		declare_constant(_T("Diffuse"),   diffuse );
		declare_constant(_T("Specular"),  specular );
		declare_constant(_T("Shininess"), shininess );

		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_linear;
		desc.addr_mode_u = address_clamp;
		desc.addr_mode_v = address_clamp;
		desc.addr_mode_w = address_clamp;
		sampler_.reset(new sampler(desc));
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
		if( tex_ )
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

class obj_loader: public quick_app{
public:
	obj_loader(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

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

        salviax_create_swap_chain_and_renderer(swap_chain_, renderer_, &render_params);
        color_surface_ = swap_chain_->get_surface();
        ds_surface_ = renderer_->create_tex2d(
            render_params.backbuffer_width,
            render_params.backbuffer_height,
            render_params.backbuffer_num_samples,
            pixel_format_color_rg32f
            )->get_surface(0);
        
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

		gen_sm_vs_ = compile_from_file("../../resources/models/sponza/sponza.obj", lang_vertex_shader);

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		
		// cup_mesh = create_mesh_from_obj( renderer_.get(), "../../resources/models/cup/cup.obj", true );

		// pps.reset( new cup_ps() );
		// pbs.reset( new bs() );
	}
	/** @} */

	void on_draw()
    {
		swap_chain_->present();
	}

	void generate_depth_texture()
	{
	}
	
	void draw()
	{
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

		generate_depth_texture();
		draw();
		
        renderer_->clear_color(color_surface_, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		renderer_->clear_depth_stencil(ds_surface_, 1.0f, 0);

		if(!cup_vs){ return; }

		static float s_angle = 0;
		s_angle -= elapsed_time * 60.0f * (static_cast<float>(TWO_PI) / 360.0f) * 0.15f;

		vec3 camera(cos(s_angle) * 2.0f, 0.5f, sin(s_angle) * 2.0f);
		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera, vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 100.0f);

		vec4 lightPos( sin( -s_angle * 1.5f) * 2.2f, 0.15f, cos(s_angle * 0.9f) * 1.8f, 0.0f );

		renderer_->set_pixel_shader(pps);
		renderer_->set_blend_shader(pbs);

		for(float i = 0 ; i < 1 ; i ++)
		{
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			renderer_->set_rasterizer_state(rs_back);

			renderer_->set_vertex_shader_code( cup_vs );
			renderer_->set_vs_variable( "wvpMatrix", &wvp );
			vec4 camera_pos = vec4( camera, 1.0f );
			renderer_->set_vs_variable( "eyePos", &camera_pos );
			renderer_->set_vs_variable( "lightPos", &lightPos );

			for( size_t i_mesh = 0; i_mesh < cup_mesh.size(); ++i_mesh )
			{
				mesh_ptr cur_mesh = cup_mesh[i_mesh];

				shared_ptr<obj_material> mtl
					= dynamic_pointer_cast<obj_material>( cur_mesh->get_attached() );
				pps->set_constant( _T("Ambient"),  &mtl->ambient );
				pps->set_constant( _T("Diffuse"),  &mtl->diffuse );
				pps->set_constant( _T("Specular"), &mtl->specular );
				pps->set_constant( _T("Shininess"),&mtl->ambient );
				// dynamic_pointer_cast<cup_ps>(pps)->set_texture(mtl->tex);

				cur_mesh->render();
			}
		}

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	swap_chain_ptr          swap_chain_;
	renderer_ptr            renderer_;
	surface_ptr				depth_surface_;
    surface_ptr             ds_surface_;
    surface_ptr             color_surface_;

	vector<mesh_ptr>        cup_mesh;

	shader_object_ptr		gen_sm_vs_;
	shader_object_ptr       plane_vs;
	shader_object_ptr       cup_vs;

	cpp_pixel_shader_ptr    pps;
	cpp_blend_shader_ptr    pbs;

	raster_state_ptr        rs_back;

	uint32_t                num_frames;
	float                   accumulate_time;
	float                   fps;

	timer                   timer;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] )
{
	obj_loader loader;
	return loader.run();
}