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

class sponza: public quick_app{
public:
	sponza(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create()
    {
		cout << "Creating window and device ..." << endl;

		string title( "Sample: Sponza" );
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
        renderer_->set_render_targets(1, &color_surface_, ds_surface_);
        
        pipeline_stat_obj_ = renderer_->create_query(async_object_ids::pipeline_statistics);
        internal_stat_obj_ = renderer_->create_query(async_object_ids::internal_statistics);

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

#ifdef SASL_VERTEX_SHADER_ENABLED
		cout << "Compiling vertex shader ... " << endl;
		sponza_sc = compile( sponza_vs_code, lang_vertex_shader );
#endif

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		cout << "Loading mesh ... " << endl;
#ifdef EFLIB_DEBUG
		cout << "Application is built in debug mode. Mesh loading is *VERY SLOW*." << endl;
#endif
		sponza_mesh = create_mesh_from_obj( renderer_.get(), "../../resources/models/sponza/sponza.obj", false );
		cout << "Loading pixel and blend shader... " << endl;

		pvs.reset( new sponza_vs() );
		pps.reset( new sponza_ps() );
		pbs.reset( new bs() );
	}
	/** @} */

	void on_draw(){
		swap_chain_->present();
	}

	void on_idle(){
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

        renderer_->begin(pipeline_stat_obj_);
        renderer_->begin(internal_stat_obj_);

        renderer_->clear_color(color_surface_, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		renderer_->clear_depth_stencil(ds_surface_, clear_depth | clear_stencil, 1.0f, 0);

		static float xpos = -36.0f;
		vec3 camera( xpos, 8.0f, 0.0f);
		vec4 camera_pos = vec4( camera, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera, vec3(40.0f, 15.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);

		static float ypos = 40.0f;
		ypos -= elapsed_time;
		if ( ypos < 1.0f ){
			ypos = 40.0f;
		}
		vec4 lightPos( 0.0f, ypos, 0.0f, 1.0f );

		renderer_->set_pixel_shader(pps);
		renderer_->set_blend_shader(pbs);

		for(float i = 0 ; i < 1 ; i ++)
		{
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			renderer_->set_rasterizer_state(rs_back);

			// C++ vertex shader and SASL vertex shader are all available.
#ifdef SASL_VERTEX_SHADER_ENABLED
			renderer_->set_vertex_shader_code( sponza_sc );
#else
			pvs->set_constant( _T("wvpMatrix"), &wvp );
			pvs->set_constant( _T("eyePos"), &camera_pos );
			pvs->set_constant( _T("lightPos"), &lightPos );
			renderer_->set_vertex_shader(pvs);
#endif
			renderer_->set_vs_variable( "wvpMatrix", &wvp );
			
			renderer_->set_vs_variable( "eyePos", &camera_pos );
			renderer_->set_vs_variable( "lightPos", &lightPos );

			for( size_t i_mesh = 0; i_mesh < sponza_mesh.size(); ++i_mesh ){
				mesh_ptr cur_mesh = sponza_mesh[i_mesh];

				shared_ptr<obj_material> mtl
					= dynamic_pointer_cast<obj_material>( cur_mesh->get_attached() );

#ifdef _DEBUG
				// if (mtl->name != "sponza_07SG"){ continue; }
#endif
				renderer_->flush();

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

				pps->set_sampler(_T("Sampler"), renderer_->create_sampler(desc, mtl->tex) );

				cur_mesh->render();
			}
		}
        
        renderer_->end(pipeline_stat_obj_);
        renderer_->end(internal_stat_obj_);

        pipeline_statistics pipeline_stat_data;
        internal_statistics internal_stat_data;
        renderer_->get_data(pipeline_stat_obj_, &pipeline_stat_data, false);
        renderer_->get_data(internal_stat_obj_, &internal_stat_data, false); 

        cout
            << "CI: " << pipeline_stat_data.cinvocations << " "
            << "CP: " << pipeline_stat_data.cprimitives << " "
            << "IAP:" << pipeline_stat_data.ia_primitives << " "
            << "IAV:" << pipeline_stat_data.ia_vertices << " "
            << "VSI:" << pipeline_stat_data.vs_invocations << " "
            << "PSI:" << pipeline_stat_data.ps_invocations << " "
            << "BIP:" << internal_stat_data.backend_input_pixels << " "
            << endl;

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	swap_chain_ptr          swap_chain_;
	renderer_ptr            renderer_;

    surface_ptr             ds_surface_;
    surface_ptr             color_surface_;

	vector<mesh_ptr>        sponza_mesh;

	shader_object_ptr       sponza_sc;

	cpp_vertex_shader_ptr	pvs;
	cpp_pixel_shader_ptr	pps;
	cpp_blend_shader_ptr	pbs;
    
    async_object_ptr        pipeline_stat_obj_;
    async_object_ptr        internal_stat_obj_;

	raster_state_ptr        rs_back;

	uint32_t                num_frames;
	float                   accumulate_time;
	float                   fps;

	timer                   timer;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] ){
	sponza loader;
	return loader.run();
}