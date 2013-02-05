#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/presenter_dev.h>
#include <salviar/include/shader.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>
#include <salviax/include/resource/texture/freeimage/tex_io_freeimage.h>
#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

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

#define SALVIA_ENABLE_PIXEL_SHADER 1

struct vert
{
	vec4 pos;
};

class vs_cone : public vertex_shader 
{
	mat44 wvp;
public:
	vs_cone():wvp(mat44::identity()){
		declare_constant(_T("WorldViewProjMat"), wvp);

		bind_semantic( "POSITION", 0, 0 );
		bind_semantic( "TEXCOORD", 0, 1 );
	}

	vs_cone(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attribute(0);
		transform(out.position(), pos, wvp);
		out.attribute(0) = in.attribute(0);//(vec4(1.0f, 1.0f, 1.0f, 1.0f) - in[0]);
		out.attribute(1) = in.attribute(1);
	}

	uint32_t num_output_attributes() const
	{
		return 2;
	}

	uint32_t output_attribute_modifiers(uint32_t index) const
	{
		switch (index)
		{
		case 0:
			return salviar::vs_output::am_linear;

		case 1:
			return salviar::vs_output::am_linear;

		case 2:
			return salviar::vs_output::am_linear;

		default:
			return salviar::vs_output::am_linear;
		}
	}
};

char const* cone_ps_code =
	"sampler samp;"
	"float4 ps_main( float4 pos: TEXCOORD0, float2 tex: TEXCOORD1 ): COLOR \r\n"
	"{\r\n"
	"	float4 color = tex2D(samp, tex); \r\n"
	"	color.w = 0.5f; \r\n"
	"	return color; \r\n"
	"}\r\n"
	;

char const* plane_ps_code =
	"sampler samp;"
	"float4 ps_main( float2 tex: TEXCOORD0 ): COLOR \r\n"
	"{\r\n"
	"	float4 color = tex2D(samp, tex); \r\n"
	"	color.w = 1.0f; \r\n" // color.w == color.a.
	"	return color; \r\n"
	"}\r\n"
	;

class ps_cone : public pixel_shader
{
	salviar::h_sampler sampler_;
	salviar::h_texture tex_;
public:

	ps_cone(const salviar::h_texture& tex)
		: tex_(tex)
	{
		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_linear;
		desc.addr_mode_u = address_clamp;
		desc.addr_mode_v = address_clamp;
		desc.addr_mode_w = address_clamp;
		sampler_.reset(new sampler(desc));
		sampler_->set_texture(tex_.get());
	}
	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f color = tex2d(*sampler_ , 1);
		color.a = 0.5;
		out.color[0] = color.get_vec4();

		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new ps_cone(*this));
	}
	virtual void destroy_clone(h_pixel_shader& ps_clone)
	{
		ps_clone.reset();
	}
};

class vs_plane : public vertex_shader
{
	mat44 wvp;
public:
	vs_plane():wvp(mat44::identity()){
		declare_constant(_T("WorldViewProjMat"), wvp);
		bind_semantic( "POSITION", 0, 0 );
	}

	vs_plane(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attribute(0);
		transform(out.position(), pos, wvp);
		out.attribute(0) = vec4(in.attribute(0).x(), in.attribute(0).z(), 0, 0);
	}

	uint32_t num_output_attributes() const
	{
		return 1;
	}

	uint32_t output_attribute_modifiers(uint32_t index) const
	{
		switch (index)
		{
		case 0:
			return salviar::vs_output::am_linear;

		default:
			return salviar::vs_output::am_linear;
		}
	}
};

class ps_plane : public pixel_shader
{
	salviar::h_sampler sampler_;
	salviar::h_texture tex_;
public:

	ps_plane(const salviar::h_texture& tex)
		: tex_(tex)
	{
		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_anisotropic;
		desc.max_anisotropy = 16;
		sampler_.reset(new sampler(desc));
		sampler_->set_texture(tex_.get());
	}

	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f color = tex2d(*sampler_, 0);
		color.a = 1;
		out.color[0] = color.get_vec4();

		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new ps_plane(*this));
	}
	virtual void destroy_clone(h_pixel_shader& ps_clone)
	{
		ps_clone.reset();
	}
};

class ts_blend_on : public blend_shader
{
public:
	bool shader_prog(size_t sample, backbuffer_pixel_out& inout, const ps_output& in)
	{
		color_rgba32f color(in.color[0]);
		inout.color(0, sample, lerp(inout.color(0, sample), color, color.a));
		return true;
	}
};

class ts_blend_off : public blend_shader
{
public:
	bool shader_prog(size_t sample, backbuffer_pixel_out& inout, const ps_output& in)
	{
		inout.color(0, sample, color_rgba32f(in.color[0]));
		return true;
	}
};

class anisotropic_filter: public quick_app{
public:
	anisotropic_filter(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		cout << "Creating window and device ..." << endl;

		string title( "Sample: Anisotropic Filter" );
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

		planar_mesh = create_planar(
			hsr.get(), 
			vec3(-50.0f, 0.0f, -50.0f), 
			vec3(1.0f, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			100, 100, true
			);
		
		cone_mesh = create_cone(hsr.get(), vec3(0.0f, 0.0f, 0.0f), 1.0f, vec3(0.0f, 1.0f, 0.0f), 120);

		pvs_cone.reset(new vs_cone());
		pvs_plane.reset(new vs_plane());

		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_anisotropic;
			desc.addr_mode_u = address_clamp;
			desc.addr_mode_v = address_clamp;
			desc.addr_mode_w = address_clamp;
			desc.max_anisotropy = 16;

			cone_tex = texture_io_fi::instance().load(hsr.get() , _T("../../resources/Dirt.jpg") , salviar::pixel_format_color_rgba8);
			cone_tex->gen_mipmap(filter_linear, true);

			pps_cone.reset(new ps_cone(cone_tex));

			cone_sampler = hsr->create_sampler( desc );
			cone_sampler->set_texture( cone_tex.get() );

#ifdef SALVIA_ENABLE_PIXEL_SHADER
			cout << "Creating Cone Pixel Shader ..." << endl;
			psc_cone = compile( cone_ps_code, lang_pixel_shader );
#endif
		}

		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_anisotropic;
			desc.max_anisotropy = 16;

			plane_tex = texture_io_fi::instance().load(hsr.get() , _T("../../resources/chessboard.png") , salviar::pixel_format_color_rgba8);
			plane_tex->gen_mipmap(filter_linear, true);
			
			pps_plane.reset(new ps_plane(plane_tex));
			plane_sampler = hsr->create_sampler( desc );
			plane_sampler->set_texture( plane_tex.get() );

#ifdef SALVIA_ENABLE_PIXEL_SHADER
			cout << "Creating Plane Pixel Shader ..." << endl;
			psc_plane = compile( plane_ps_code, lang_pixel_shader );
#endif
		}

		pbs_cone.reset(new ts_blend_on);
		pbs_plane.reset(new ts_blend_off);

		rasterizer_desc rs_desc;
		rs_desc.cm = cull_front;
		rs_front.reset(new rasterizer_state(rs_desc));
		rs_desc.cm = cull_back;
		rs_back.reset(new rasterizer_state(rs_desc));

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;
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

		mat44 world(mat44::identity()), view, proj, wvp;
		
		mat_lookat(view, vec3(0.0f, 1.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 100.0f);

		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;

		static int total_time = 0;
		total_time += (int)( elapsed_time * 1000 );
		if(total_time/3000 > 4) { total_time = 0; }

		if( total_time/3000 == 0 )
		{
			desc.mip_filter = filter_linear;
			desc.max_anisotropy = 0;
			impl->main_window()->set_title( std::string("Sample: Anisotropic Filter - Mipmap") );
		}
		else
		{
			desc.mip_filter = filter_anisotropic;
			desc.max_anisotropy = 1 << (total_time/3000);
			impl->main_window()->set_title( ( boost::format("Sample: Anisotropic Filter - AF %dX") % desc.max_anisotropy ).str() );
		}
		plane_sampler->set_sampler_desc(desc);

		for(float i = 0 ; i < 1 ; i ++)
		{	
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			hsr->set_rasterizer_state(rs_back);
			pvs_plane->set_constant(_T("WorldViewProjMat"), &wvp);
			hsr->set_vertex_shader(pvs_plane);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			hsr->set_pixel_shader_code( psc_plane );
			hsr->set_ps_sampler( "samp", plane_sampler );
#else
			hsr->set_pixel_shader(pps_plane);
#endif
			hsr->set_blend_shader(pbs_plane);
			planar_mesh->render();
			
			/*
			hsr->set_rasterizer_state(rs_front);
			pvs_cone->set_constant(_T("WorldViewProjMat"), &wvp);
			hsr->set_vertex_shader(pvs_cone);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			hsr->set_pixel_shader_code( psc_cone );
			hsr->set_ps_sampler( "samp", cone_sampler );
#else
			hsr->set_pixel_shader(pps_cone);
#endif
			hsr->set_blend_shader(pbs_cone);
			cone_mesh->render();

			hsr->set_rasterizer_state(rs_back);
			hsr->set_vertex_shader(pvs_cone);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			hsr->set_pixel_shader_code( psc_cone );
			hsr->set_ps_sampler( "samp", cone_sampler );
#else
			hsr->set_pixel_shader(pps_cone);
#endif
			hsr->set_blend_shader(pbs_cone);
			cone_mesh->render();
			*/
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
	h_mesh cone_mesh;

	h_texture plane_tex;
	h_texture cone_tex;

	h_sampler plane_sampler;
	h_sampler cone_sampler;

	h_vertex_shader pvs_cone;
	h_pixel_shader pps_cone;
	h_shader_code psc_cone;

	h_vertex_shader pvs_plane;
	h_pixel_shader pps_plane;
	h_shader_code psc_plane;

	h_blend_shader pbs_cone;
	h_blend_shader pbs_plane;

	h_rasterizer_state rs_front;
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
	anisotropic_filter loader;
	return loader.run();
}