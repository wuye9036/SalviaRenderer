#include <salviar/include/shader.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/swap_chain/swap_chain.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>

#include <salviau/include/common/sample_app.h>
#include <eflib/include/platform/main.h>

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
// #define SALVIA_PIXEL_SHADER_ENABLED

char const* vs_code =
"float4x4 wvpMatrix; \r\n"
"struct VSIn{ \r\n"
"	float4 pos: POSITION; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: sv_position; \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
"	out.pos = mul(in.pos, wvpMatrix); \r\n"
"	return out; \r\n"
"} \r\n"
;

char const* ps_code =
"float4 color; \r\n"
"float4 ps_main(): COLOR \r\n"
"{ \r\n"
"	return color; \r\n"
"} \r\n"
;

class ps : public cpp_pixel_shader
{
public:
	vec4 color;
	ps()
	{
		declare_constant(_T("Color"),   color );
	}
	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		out.color[0] = color;
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

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 30 : 2500;
int const TEST_FRAME_COUNT		= 2;

class triangles: public sample_app
{
public:
	triangles(): sample_app("Triangles") {}

protected:
	void on_init() override
	{
		create_devices_and_targets(data_->screen_width, data_->screen_height, 1, pixel_format_color_bgra8, pixel_format_color_rg32f);
		data_->renderer->set_viewport(data_->screen_vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_none;
		rs_back.reset(new raster_state(rs_desc));

		cout << "Compiling vertex shader ... " << endl;
		vsc = compile( vs_code, lang_vertex_shader );
		data_->renderer->set_vertex_shader_code( vsc );

		cout << "Compiling pixel shader ... " << endl;
		psc = compile( ps_code, lang_pixel_shader );
#ifdef SALVIA_PIXEL_SHADER_ENABLED
		data_->renderer->set_pixel_shader_code(psc);
#endif
		mesh_ptr pmesh;

		pmesh = create_planar(
			data_->renderer.get(), 
			vec3(-30.0f, -1.0f, -30.0f), 
			vec3(15.0f, 0.0f, 1.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			4, 60, false
			);
		meshes.push_back( pmesh );

		pmesh = create_planar(
			data_->renderer.get(), 
			vec3(-5.0f, -5.0f, -30.0f), 
			vec3(0.0f, 4.0f, 0.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			4, 60, false
			);

		meshes.push_back( pmesh );

		pmesh = create_box( data_->renderer.get() );
		meshes.push_back( pmesh );

		pps.reset( new ps() );
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

	void on_frame()
	{
		profiling("BackBufferClearing", [this](){
			data_->renderer->clear_color(data_->color_target, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
			data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth | clear_stencil, 1.0f, 0);
		});

		static float t = 17.08067f;
		float r = 2.0f * ( 1 + t );
		float x = r * cos(t);
		float y = r * sin(t);

		vec3 camera( x, 8.0f, y);
		vec4 camera_pos = vec4( camera, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;
		mat_scale( world, 30.0f, 30.0f, 30.0f );
		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), data_->screen_aspect_ratio, 0.1f, 1000.0f);
		mat_mul(wvp, world, mat_mul(wvp, view, proj));

		data_->renderer->set_pixel_shader(pps);
		data_->renderer->set_blend_shader(pbs);
		data_->renderer->set_rasterizer_state(rs_back);

		profiling("Rendering", [&](){
			// C++ vertex shader and SASL vertex shader are all available.
			data_->renderer->set_vertex_shader_code( vsc );
			data_->renderer->set_vs_variable( "wvpMatrix", &wvp );

#ifdef SALVIA_PIXEL_SHADER_ENABLED
			data_->renderer->set_pixel_shader_code(psc);
#endif
			vec4 color[3];
			color[0] = vec4( 0.3f, 0.7f, 0.3f, 1.0f );
			color[1] = vec4( 0.3f, 0.3f, 0.7f, 1.0f );
			color[2] = vec4( 0.7f, 0.3f, 0.3f, 1.0f );

			for( size_t i_mesh = 0; i_mesh < meshes.size(); ++i_mesh )
			{
				mesh_ptr cur_mesh = meshes[i_mesh];
				data_->renderer->set_ps_variable( "color", &color[i_mesh] );
				pps->set_constant( _T("Color"), &color[i_mesh] );
				cur_mesh->render();
			}
		});
	}

protected:
	vector<mesh_ptr>		meshes;
	shader_object_ptr       vsc;
	shader_object_ptr       psc;

	cpp_vertex_shader_ptr	pvs;
	cpp_pixel_shader_ptr	pps;
	cpp_blend_shader_ptr	pbs;

	raster_state_ptr        rs_back;
};

EFLIB_MAIN(argc, argv)
{
	triangles loader;
	loader.init(argc, const_cast<std::_tchar const**>(argv));
	loader.run();

	return 0;
}
