#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_impl.h>
#include <salviax/include/swap_chain/swap_chain.h>

#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>
#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>

#include <eflib/include/utility/unref_declarator.h>
#include <eflib/include/diagnostics/profiler.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/assign.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>
#include <fstream>
#include <string>

#include <tchar.h>

using namespace eflib;
using namespace boost;
using namespace boost::assign;
using namespace std;
using namespace salviar;
using namespace salviax;
using namespace salviax::resource;
using namespace salviau;

char const* vs_code = 
"float4x4	wvpMatrix; \r\n"
"float4		lightPos0; \r\n"
"float4		lightPos1; \r\n"
"float4		lightPos2; \r\n"
"struct VSIn{ \r\n"
"	float4 pos: POSITION; \r\n"
"	float4 norm: NORMAL; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: sv_position; \r\n"
"	float4 norm: TEXCOORD(0); \r\n"
"	float4 lightDir0: TEXCOORD(1); \r\n"
"	float4 lightDir1: TEXCOORD(2); \r\n"
"	float4 lightDir2: TEXCOORD(3); \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
"	out.norm = in.norm; \r\n"
"	out.pos = mul(in.pos, wvpMatrix); \r\n"
"	out.lightDir0 = lightPos0 - in.pos;"
"	out.lightDir1 = lightPos1 - in.pos;"
"	out.lightDir2 = lightPos2 - in.pos;"
"	return out; \r\n"
"} \r\n"
;

class ps : public cpp_pixel_shader
{
public:
	ps()
	{}
	bool shader_prog(const vs_output& in, ps_output& out)
	{
		vec3 lightDir0 = in.attribute(1).xyz();
		vec3 lightDir1 = in.attribute(2).xyz();
		vec3 lightDir2 = in.attribute(3).xyz();

		vec3 norm = in.attribute(0).xyz();

		float invLight0Distance = 1.0f / lightDir0.length();
		float invLight1Distance = 1.0f / lightDir1.length();
		float invLight2Distance = 1.0f / lightDir2.length();

		vec3 normalized_norm = normalize3( norm );
		vec3 normalized_lightDir0 = lightDir0 * invLight0Distance;
		vec3 normalized_lightDir1 = lightDir1 * invLight1Distance;
		vec3 normalized_lightDir2 = lightDir2 * invLight2Distance;

		float refl0 = dot_prod3( normalized_norm, normalized_lightDir0 );
		float refl1 = dot_prod3( normalized_norm, normalized_lightDir1 );
		float refl2 = dot_prod3( normalized_norm, normalized_lightDir2 );

		out.color[0] = clampss(
			vec4(0.7f, 0.1f, 0.3f, 1.0f ) * refl0 * invLight0Distance * invLight0Distance +
			vec4(0.1f, 0.3f, 0.7f, 1.0f ) * refl1 * invLight1Distance * invLight1Distance +
			vec4(0.3f, 0.7f, 0.1f, 1.0f ) * refl2 * invLight2Distance * invLight2Distance
			, 0.0f, 1.0f
			)
			;

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

class colorized_triangle : public quick_app
{
public:
	colorized_triangle()
		: quick_app( create_wtl_application() )
		, num_frames(0)
		, accumulate_time(0.0f)
		, fps(0.0f)
	{
	}
	
	virtual void on_create()
	{
		cout << "Creating window and device ..." << endl;

		string title( "Sample: Colorized Triangle" );
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

		shader_object_ptr compiled_code;
		compiled_code = compile( vs_code, lang_vertex_shader );

		renderer_->set_vertex_shader_code( compiled_code );

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		planar_mesh = create_planar(
			renderer_.get(), 
			vec3(-3.0f, -1.0f, -3.0f), 
			vec3(6, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 6),
			1, 1, false
			);

		pps.reset( new ps() );
		pbs.reset( new bs() );
	}

	void on_draw()
    {
		swap_chain_->present();
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

        renderer_->clear_color(color_surface_, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		renderer_->clear_depth_stencil(ds_surface_, clear_depth | clear_stencil, 1.0f, 0);

		static float s_angle = 0;
		s_angle -= elapsed_time * 60.0f * (static_cast<float>(TWO_PI) / 360.0f) * 0.15f;

		vec3 camera(cos(s_angle) * 2.3f, 2.5f, sin(s_angle) * 2.3f);
		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 100.0f);

		vec4 lightPos0( sin( -s_angle * 1.5f) * 2.2f, 0.15f, cos(s_angle * 0.9f) * 1.8f, 0.0f );
		vec4 lightPos1( sin(s_angle * 0.7f) * 1.9f, 0.15f, cos( -s_angle * 0.4f) * 2.5f, 0.0f );
		vec4 lightPos2( sin(s_angle * 2.6f) * 2.3f, 0.15f, cos(s_angle * 0.6f) * 1.7f, 0.0f );
		for(float i = 0 ; i < 1 ; i ++)
		{
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			renderer_->set_rasterizer_state(rs_back);

			renderer_->set_vs_variable( "wvpMatrix", &wvp );
			
			renderer_->set_vs_variable( "lightPos0", &lightPos0 );
			renderer_->set_vs_variable( "lightPos1", &lightPos1 );
			renderer_->set_vs_variable( "lightPos2", &lightPos2 );

			renderer_->set_pixel_shader(pps);
			renderer_->set_blend_shader(pbs);
			planar_mesh->render();
		}

		impl->main_window()->refresh();
	}
	
private:
	swap_chain_ptr			swap_chain_;
	renderer_ptr			renderer_;
	mesh_ptr				planar_mesh;
    
    surface_ptr             ds_surface_;
    surface_ptr             color_surface_;

	cpp_pixel_shader_ptr	pps;
	cpp_blend_shader_ptr	pbs;

	raster_state_ptr		rs_back;

	uint32_t				num_frames;
	float					accumulate_time;
	float					fps;

	timer					timer;
};

int main( int /*argc*/, TCHAR* /*argv*/[] )
{
    colorized_triangle loader;
	return loader.run();
}
