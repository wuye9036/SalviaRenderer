#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/presenter_dev.h>
#include <salviar/include/shader.h>
#include <salviar/include/shader_code.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>

#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#include <vector>

#if defined( SALVIA_BUILD_WITH_DIRECTX )
#define PRESENTER_NAME "d3d9"
#else
#define PRESENTER_NAME "opengl"
#endif

using namespace eflib;
using namespace salviar;
using namespace salviax;
using namespace salviax::resource;
using namespace salviau;

using boost::shared_ptr;
using boost::shared_polymorphic_cast;

using std::string;
using std::vector;
using std::cout;
using std::endl;

#define SASL_VERTEX_SHADER_ENABLED
#define SALVIA_PIXEL_SHADER_ENABLED

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

class ps : public pixel_shader
{
public:
	vec4 color;
	ps()
	{
		declare_constant(_T("Color"),   color );
	}
	bool shader_prog(const vs_output& in, ps_output& out)
	{
		out.color[0] = color;
		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new ps(*this));
	}
	virtual void destroy_clone(h_pixel_shader& ps_clone)
	{
		ps_clone.reset();
	}
};

class bs : public blend_shader
{
public:
	bool shader_prog(size_t sample, backbuffer_pixel_out& inout, const ps_output& in)
	{
		color_rgba32f color(in.color[0]);
		inout.color( 0, sample, color_rgba32f(in.color[0]) );
		return true;
	}
};

class triangles: public quick_app{
public:
	triangles(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		cout << "Creating window and device ..." << endl;

		string title( "Sample: Triangles" );
		impl->main_window()->set_title( title );

		std::_tstring dll_name = TEXT("salviax_");
		dll_name += TEXT(PRESENTER_NAME);
		dll_name += TEXT("_presenter");
#ifdef EFLIB_DEBUG
		dll_name += TEXT("_d");
#endif
		dll_name += TEXT(".dll");

		HMODULE presenter_dll = LoadLibrary(dll_name.c_str());
		typedef void (*create_presenter_device_func)(salviar::h_device& dev, void* param);
		create_presenter_device_func presenter_func = (create_presenter_device_func)GetProcAddress(presenter_dll, "salviax_create_presenter_device");
		
		boost::any view_handle_any = impl->main_window()->view_handle();
		presenter_func(present_dev, *boost::unsafe_any_cast<void*>( &view_handle_any ) );

		renderer_parameters render_params = {0};
		render_params.backbuffer_format = pixel_format_color_bgra8;
		render_params.backbuffer_height = 512;
		render_params.backbuffer_width = 512;
		render_params.backbuffer_num_samples = 1;

		hsr = create_software_renderer(&render_params, present_dev);

		const h_framebuffer& fb = hsr->get_framebuffer();
		if (fb->get_num_samples() > 1){
			display_surf.reset(new surface(fb->get_width(),
				fb->get_height(), 1, fb->get_buffer_format()));
			pdsurf = display_surf.get();
		}
		else{
			display_surf.reset();
			pdsurf = fb->get_render_target(render_target_color, 0);
		}

		rasterizer_desc rs_desc;
		rs_desc.cm = cull_none;
		rs_back.reset(new rasterizer_state(rs_desc));

		cout << "Compiling vertex shader ... " << endl;
		vsc = shader_code::create( vs_code, lang_vertex_shader );
		hsr->set_vertex_shader_code( vsc );

		cout << "Compiling pixel shader ... " << endl;
		psc = shader_code::create( ps_code, lang_pixel_shader );
#ifdef SALVIA_PIXEL_SHADER_ENABLED
		hsr->set_pixel_shader_code(psc);
#endif

		h_mesh pmesh = create_planar(
			hsr.get(), 
			vec3(-30.0f, -1.0f, -30.0f), 
			vec3(15.0f, 0.0f, 1.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			4, 60, false
			);
		meshes.push_back( pmesh );

		pmesh = create_planar(
			hsr.get(), 
			vec3(-5.0f, -5.0f, -30.0f), 
			vec3(0.0f, 4.0f, 0.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			4, 60, false
			);

		meshes.push_back( pmesh );

		pmesh = create_box( hsr.get() );
		meshes.push_back( pmesh );

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		pps.reset( new ps() );
		pbs.reset( new bs() );
	}
	/** @} */

	void on_draw(){
		present_dev->present(*pdsurf);
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

		hsr->clear_color(0, color_rgba32f(0.05f, 0.05f, 0.2f, 1.0f));
		hsr->clear_depth(1.0f);

		static float t = 17.08067f;
		// t += elapsed_time / 50000.0f;
		float r = 2.0f * ( 1 + t );
		float x = r * cos(t);
		float y = r * sin(t);

		vec3 camera( x, 8.0f, y);
		vec4 camera_pos = vec4( camera, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;
		mat_scale( world, 30.0f, 30.0f, 30.0f );
		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);

		static float ypos = 40.0f;
		ypos -= elapsed_time;
		if ( ypos < 1.0f ){
			ypos = 40.0f;
		}

		hsr->set_pixel_shader(pps);
		hsr->set_blend_shader(pbs);

		for(float i = 0 ; i < 1 ; i ++)
		{
			// mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			hsr->set_rasterizer_state(rs_back);

			// C++ vertex shader and SASL vertex shader are all available.
			hsr->set_vertex_shader_code( vsc );
			hsr->set_vs_variable( "wvpMatrix", &wvp );

#ifdef SALVIA_PIXEL_SHADER_ENABLED
			hsr->set_pixel_shader_code(psc);
#endif
			vec4 color[3];
			color[0] = vec4( 0.3f, 0.7f, 0.3f, 1.0f );
			color[1] = vec4( 0.3f, 0.3f, 0.7f, 1.0f );
			color[2] = vec4( 0.7f, 0.3f, 0.3f, 1.0f );

			for( size_t i_mesh = 0; i_mesh < meshes.size(); ++i_mesh ){
				h_mesh cur_mesh = meshes[i_mesh];
				hsr->set_ps_variable( "color", &color[i_mesh] );
				pps->set_constant( _T("Color"), &color[i_mesh] );
				cur_mesh->render();
			}
		}

		if (hsr->get_framebuffer()->get_num_samples() > 1){
			hsr->get_framebuffer()->get_render_target(render_target_color, 0)->resolve(*display_surf);
		}

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	h_device present_dev;
	h_renderer hsr;

	vector<h_mesh>			meshes;
	shared_ptr<shader_code> vsc;
	shared_ptr<shader_code> psc;

	h_vertex_shader	pvs;
	h_pixel_shader	pps;
	h_blend_shader	pbs;

	h_rasterizer_state rs_back;

	h_surface display_surf;
	surface* pdsurf;

	uint32_t num_frames;
	float accumulate_time;
	float fps;

	timer_t timer;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] ){
	triangles loader;
	return loader.run();
}