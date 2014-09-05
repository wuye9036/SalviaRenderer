#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_impl.h>
#include <salviax/include/swap_chain/swap_chain.h>

#include <salviau/include/common/sample_app.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>

#include <eflib/include/utility/unref_declarator.h>
#include <eflib/include/diagnostics/profiler.h>
#include <eflib/include/platform/main.h>

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

int const TEST_TOTAL_FRAME_COUNT = 5;

#if defined(EFLIB_DEBUG)
	int const BENCHMARK_TOTAL_FRAME_COUNT = 3;	
#else
	int const BENCHMARK_TOTAL_FRAME_COUNT = 2000;
#endif

class colorized_triangle : public sample_app
{
public:
	colorized_triangle(): sample_app("Colorized Triangle")
	{
	}
	
	void on_init() override
	{
		create_devices_and_targets(512, 512, 1, pixel_format_color_rgba8, pixel_format_color_rg32f);

		viewport vp = {0, 0, 512, 512, 0.0f, 1.0f};
        data_->renderer->set_viewport(vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

		shader_object_ptr compiled_code;
		compiled_code = compile( vs_code, lang_vertex_shader );

		data_->renderer->set_vertex_shader_code(compiled_code);

		mesh_ = create_planar(
			data_->renderer.get(), 
			vec3(-3.0f, -1.0f, -3.0f), 
			vec3(6, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 6),
			1, 1, false
			);

		pps.reset( new ps() );
		pbs.reset( new bs() );

		camera_angle = 0.0f;
	}

	void on_frame() override
	{
		switch(data_->mode)
		{
		case salviau::app_modes::benchmark:
			data_->quiting = (data_->frame_count == BENCHMARK_TOTAL_FRAME_COUNT);
			break;
		case salviau::app_modes::test:
			data_->quiting = (data_->frame_count == TEST_TOTAL_FRAME_COUNT);
			break;
		}

		if(data_->quiting)
		{
			return;
		}

		profiling("BackBufferClearing", [this]()
		{
			data_->renderer->clear_color(data_->color_target, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
			data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth | clear_stencil, 1.0f, 0);
		});

		switch(data_->mode)
		{
		case salviau::app_modes::benchmark:
			camera_angle -= 0.01f;
			break;
		case salviau::app_modes::test:
			camera_angle -= 0.55f;
			break;
		case salviau::app_modes::interactive:
			break;
		case salviau::app_modes::replay:
			camera_angle -= static_cast<float>(data_->elapsed_sec * 60.0f * TWO_PI / 360.0f * 0.15f);
			break;
		}

		vec3 camera(cos(camera_angle) * 2.3f, 2.5f, sin(camera_angle) * 2.3f);
		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 100.0f);

		vec4 lightPos0( sin(-camera_angle * 1.5f) * 2.2f, 0.15f, cos( camera_angle * 0.9f) * 1.8f, 0.0f );
		vec4 lightPos1( sin( camera_angle * 0.7f) * 1.9f, 0.15f, cos(-camera_angle * 0.4f) * 2.5f, 0.0f );
		vec4 lightPos2( sin( camera_angle * 2.6f) * 2.3f, 0.15f, cos( camera_angle * 0.6f) * 1.7f, 0.0f );
		
		mat_translate(world , -0.5f, 0, -0.5f);
		mat_mul(wvp, world, mat_mul(wvp, view, proj));

		data_->renderer->set_rasterizer_state(rs_back);

		profiling("Rendering", [&](){
			data_->renderer->set_vs_variable( "wvpMatrix", &wvp );
			
			data_->renderer->set_vs_variable( "lightPos0", &lightPos0 );
			data_->renderer->set_vs_variable( "lightPos1", &lightPos1 );
			data_->renderer->set_vs_variable( "lightPos2", &lightPos2 );

			data_->renderer->set_pixel_shader(pps);
			data_->renderer->set_blend_shader(pbs);

			mesh_->render();
		});
	}

private:
	mesh_ptr				mesh_;
	cpp_pixel_shader_ptr	pps;
	cpp_blend_shader_ptr	pbs;
	raster_state_ptr		rs_back;

	float					camera_angle;
};

EFLIB_MAIN(argc, argv)
{
    colorized_triangle loader;
	loader.init( argc, const_cast<std::_tchar const**>(argv) );
	loader.run();

	return 0;
}
