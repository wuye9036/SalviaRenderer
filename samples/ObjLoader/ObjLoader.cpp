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

#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#define PRESENTER_NAME "d3d9"

using namespace eflib;
using namespace salviar;
using namespace salviax;
using namespace salviax::resource;
using namespace salviau;

using boost::shared_ptr;
using boost::static_pointer_cast;

using std::string;
using std::cout;
using std::endl;

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

class ps : public pixel_shader
{
public:
	ps()
	{}
	bool shader_prog(const vs_output& in, ps_output& out)
	{
		vec3 lightDir0 = in.attributes[1].xyz();
		vec3 lightDir1 = in.attributes[2].xyz();
		vec3 lightDir2 = in.attributes[3].xyz();

		vec3 norm = in.attributes[0].xyz();

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

class obj_loader: public quick_app{
public:
	obj_loader(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		string title( "Sample: Obj File Loader" );
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
		rs_desc.cm = cull_back;
		rs_back.reset(new rasterizer_state(rs_desc));

		shared_ptr<shader_code> compiled_code;
		salvia_create_shader( compiled_code, vs_code, lang_vertex_shader );

		hsr->set_vertex_shader_code( compiled_code );

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		planar_mesh = create_planar(
			hsr.get(), 
			vec3(-3.0f, -1.0f, -3.0f), 
			vec3(6, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 6),
			1, 1, false
			);

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

		hsr->clear_color(0, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		hsr->clear_depth(1.0f);

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

			hsr->set_rasterizer_state(rs_back);

			hsr->set_vs_variable( "wvpMatrix", &wvp );
			
			hsr->set_vs_variable( "lightPos0", &lightPos0 );
			hsr->set_vs_variable( "lightPos1", &lightPos1 );
			hsr->set_vs_variable( "lightPos2", &lightPos2 );

			hsr->set_pixel_shader(pps);
			hsr->set_blend_shader(pbs);
			planar_mesh->render();
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

	h_texture sm_tex;
	h_mesh planar_mesh;

	h_pixel_shader pps;
	h_blend_shader pbs;

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
	obj_loader loader;
	return loader.run();
}