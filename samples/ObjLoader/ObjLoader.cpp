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

#if defined(SALVIA_BUILD_WITH_DIRECTX)
#	define PRESENTER_NAME "d3d9"
#else
#	define PRESENTER_NAME "opengl"
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

class cup_ps : public pixel_shader
{
	salviar::h_sampler sampler_;
	salviar::h_texture tex_;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	int shininess;
public:
	void set_texture( salviar::h_texture tex ){
		tex_ = tex;
		sampler_->set_texture(tex_.get());
	}

	cup_ps()
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

	bool shader_prog(const vs_output& in, ps_output& out)
	{
		color_rgba32f tex_color(1.0f, 1.0f, 1.0f, 1.0f);
		if( tex_ ){
			tex_color = tex2d(*sampler_ , 0);
		}
		vec3 norm( normalize3( in.attributes[1].xyz() ) );
		vec3 light_dir( normalize3( in.attributes[2].xyz() ) );
		vec3 eye_dir( normalize3( in.attributes[3].xyz() ) );

		float illum_diffuse = clamp( dot_prod3( light_dir, norm ), 0.0f, 1.0f );
		float illum_specular = clamp( dot_prod3( reflect3( light_dir, norm ), eye_dir ), 0.0f, 1.0f );
		vec4 illum = ambient + diffuse * illum_diffuse + specular * illum_specular;

		out.color[0] = tex_color.get_vec4() * illum;
		out.color[0][3] = 1.0f;

		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new cup_ps(*this));
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

		cup_vs = shader_code::create( cup_vs_code, lang_vertex_shader );

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		cup_mesh = create_mesh_from_obj( hsr.get(), "../../resources/models/cup/cup.obj", true );

		pps.reset( new cup_ps() );
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

		vec3 camera(cos(s_angle) * 2.0f, 0.5f, sin(s_angle) * 2.0f);
		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera, vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 100.0f);

		vec4 lightPos( sin( -s_angle * 1.5f) * 2.2f, 0.15f, cos(s_angle * 0.9f) * 1.8f, 0.0f );

		hsr->set_pixel_shader(pps);
		hsr->set_blend_shader(pbs);

		for(float i = 0 ; i < 1 ; i ++)
		{
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			hsr->set_rasterizer_state(rs_back);

			hsr->set_vertex_shader_code( cup_vs );
			hsr->set_vs_variable( "wvpMatrix", &wvp );
			vec4 camera_pos = vec4( camera, 1.0f );
			hsr->set_vs_variable( "eyePos", &camera_pos );
			hsr->set_vs_variable( "lightPos", &lightPos );

			for( size_t i_mesh = 0; i_mesh < cup_mesh.size(); ++i_mesh ){
				h_mesh cur_mesh = cup_mesh[i_mesh];

				shared_ptr<obj_material> mtl
					= shared_polymorphic_cast<obj_material>( cur_mesh->get_attached() );
				pps->set_constant( _T("Ambient"),  &mtl->ambient );
				pps->set_constant( _T("Diffuse"),  &mtl->diffuse );
				pps->set_constant( _T("Specular"), &mtl->specular );
				pps->set_constant( _T("Shininess"),&mtl->ambient );
				shared_polymorphic_cast<cup_ps>( pps )->set_texture( mtl->tex );

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

	vector<h_mesh> cup_mesh;

	shared_ptr<shader_code> plane_vs;
	shared_ptr<shader_code> cup_vs;

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