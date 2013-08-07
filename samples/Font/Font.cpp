#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/shader.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/resource/font/font.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>
#include <salviax/include/resource/texture/freeimage/tex_io_freeimage.h>

#include <salviau/include/common/timer.h>
#include <salviau/include/common/presenter_utility.h>
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

// #define SALVIA_ENABLE_PIXEL_SHADER 1

struct vert
{
	vec4 pos;
};

class vs_box : public cpp_vertex_shader
{
	mat44 wvp;
public:
	vs_box():wvp(mat44::identity()){
		declare_constant(_T("WorldViewProjMat"), wvp);

		bind_semantic( "POSITION", 0, 0 );
		bind_semantic( "TEXCOORD", 0, 1 );
	}

	vs_box(const mat44& wvp):wvp(wvp){}
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

char const* box_ps_code =
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
	

class ps_box : public cpp_pixel_shader
{
	salviar::sampler_ptr sampler_;
	salviar::texture_ptr tex_;
public:

	ps_box(const salviar::texture_ptr& tex)
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
	virtual cpp_pixel_shader_ptr create_clone()
	{
		return cpp_pixel_shader_ptr(new ps_box(*this));
	}
	virtual void destroy_clone(cpp_pixel_shader_ptr& ps_clone)
	{
		ps_clone.reset();
	}
};

class vs_plane : public cpp_vertex_shader
{
	mat44 wvp;
public:
	vs_plane():wvp(mat44::identity()){
		declare_constant(_T("WorldViewProjMat"), wvp);
		bind_semantic( "POSITION", 0, 0 );
		bind_semantic( "TEXCOORD", 0, 1 );
	}

	vs_plane(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attribute(0);
		transform(out.position(), pos, wvp);
		out.attribute(0) = in.attribute(1); // vec4(in.attribute(0).x(), in.attribute(0).z(), 0, 0);
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

class ps_plane : public cpp_pixel_shader
{
	salviar::sampler_ptr sampler_;
	salviar::texture_ptr tex_;
public:

	ps_plane(const salviar::texture_ptr& tex)
		: tex_(tex)
	{
		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_linear;
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
	virtual cpp_pixel_shader_ptr create_clone()
	{
		return cpp_pixel_shader_ptr(new ps_plane(*this));
	}
	virtual void destroy_clone(cpp_pixel_shader_ptr& ps_clone)
	{
		ps_clone.reset();
	}
};

class ts_blend_on : public cpp_blend_shader
{
public:
	bool shader_prog(size_t sample, pixel_accessor& inout, const ps_output& in)
	{
		color_rgba32f color(in.color[0]);
		inout.color(0, sample, lerp(inout.color(0, sample), color, color.a));
		return true;
	}
};

class ts_blend_off : public cpp_blend_shader
{
public:
	bool shader_prog(size_t sample, pixel_accessor& inout, const ps_output& in)
	{
		inout.color(0, sample, color_rgba32f(in.color[0]));
		return true;
	}
};

class font_sample: public quick_app{
public:
	font_sample(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		cout << "Creating window and device ..." << endl;

		string title( "Sample: Font" );
		impl->main_window()->set_title(title);
		boost::any view_handle_any = impl->main_window()->view_handle();
		present_dev = create_default_presenter( *boost::unsafe_any_cast<void*>(&view_handle_any) );
		
		renderer_parameters render_params = {0};
		render_params.backbuffer_format = pixel_format_color_bgra8;
		render_params.backbuffer_height = 512;
		render_params.backbuffer_width = 512;
		render_params.backbuffer_num_samples = 1;

		hsr = create_software_renderer(&render_params, present_dev);

		const framebuffer_ptr& fb = hsr->get_framebuffer();
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
			vec3(-1.0f, -1.0f, 0.0f), 
			vec3(1.0f, 0.0f, 0.0f), 
			vec3(0.0f, 1.0f, 0.0f),
			2, 2, true
			);
		
		box_mesh = create_box(hsr.get());

		pvs_box.reset(new vs_box());
		pvs_plane.reset(new vs_plane());

		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_linear;
			desc.addr_mode_u = address_clamp;
			desc.addr_mode_v = address_clamp;
			desc.addr_mode_w = address_clamp;

			box_tex = texture_io_fi::instance().load(hsr.get() , _T("../../resources/Dirt.jpg") , salviar::pixel_format_color_rgba8);
			box_tex->gen_mipmap(filter_linear, true);

			pps_box.reset(new ps_box(box_tex));

			box_sampler = hsr->create_sampler( desc );
			box_sampler->set_texture( box_tex.get() );

#ifdef SALVIA_ENABLE_PIXEL_SHADER
			cout << "Creating Box Pixel Shader ..." << endl;
			psc_box = compile( box_ps_code, lang_pixel_shader );
#endif
		}

		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_linear;
			desc.addr_mode_u = address_wrap;
			desc.addr_mode_v = address_wrap;

			plane_tex = hsr->create_tex2d(512, 512, 1, pixel_format_color_rgba8);
			fnt = font::create_in_system_path("msyh.ttc", 0, 14, font::points);
			fnt->draw( "HelloÄãºÃ123", plane_tex->get_surface(0).get(), rect<int32_t>(0, 0, 512, 512),
				color_rgba32f(0.8f, 0.8f, 1.0f, 1.0f), color_rgba32f(0.0f, 0.0f, 0.0f, 1.0f), font::antialias );
			plane_tex->gen_mipmap(filter_linear, true);
			
			pps_plane.reset(new ps_plane(plane_tex));
			plane_sampler = hsr->create_sampler( desc );
			plane_sampler->set_texture( plane_tex.get() );

#ifdef SALVIA_ENABLE_PIXEL_SHADER
			cout << "Creating Plane Pixel Shader ..." << endl;
			psc_plane = compile( plane_ps_code, lang_pixel_shader );
#endif
		}

		pbs_box.reset(new ts_blend_on);
		pbs_plane.reset(new ts_blend_off);

		raster_desc rs_desc;
		rs_desc.cm = cull_front;
		rs_front.reset(new raster_state(rs_desc));
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

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

		//static float s_angle = 0;
		//s_angle -= elapsed_time * 60.0f * (static_cast<float>(TWO_PI) / 360.0f);
		//vec3 camera(cos(s_angle) * 1.5f, 1.5f, sin(s_angle) * 1.5f);
		vec3 camera(0.0f, 0.0f, -2.2f);

		mat44 world(mat44::identity()), view, proj, wvp;
		
		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_ortho(proj, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1000.0f);
		// mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 100.0f);

		for(float i = 0 ; i < 1 ; i ++)
		{
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
			pvs_box->set_constant(_T("WorldViewProjMat"), &wvp);
			hsr->set_vertex_shader(pvs_box);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			hsr->set_pixel_shader_code( psc_box );
			hsr->set_ps_sampler( "samp", box_sampler );
#else
			hsr->set_pixel_shader(pps_box);
#endif
			hsr->set_blend_shader(pbs_box);
			box_mesh->render();
			
			hsr->set_rasterizer_state(rs_back);
			hsr->set_vertex_shader(pvs_box);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			hsr->set_pixel_shader_code( psc_box );
			hsr->set_ps_sampler( "samp", box_sampler );
#else
			hsr->set_pixel_shader(pps_box);
#endif
			hsr->set_blend_shader(pbs_box);
			box_mesh->render();
			*/
		}

		if (hsr->get_framebuffer()->get_num_samples() > 1){
			hsr->get_framebuffer()->get_render_target(render_target_color, 0)->resolve(*display_surf);
		}

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	device_ptr present_dev;
	renderer_ptr hsr;
	texture_ptr sm_tex;

	mesh_ptr planar_mesh;
	mesh_ptr box_mesh;

	texture_ptr plane_tex;
	texture_ptr box_tex;

	sampler_ptr plane_sampler;
	sampler_ptr box_sampler;

	cpp_vertex_shader_ptr pvs_box;
	cpp_pixel_shader_ptr pps_box;
	shader_object_ptr psc_box;

	cpp_vertex_shader_ptr pvs_plane;
	cpp_pixel_shader_ptr pps_plane;
	shader_object_ptr psc_plane;

	cpp_blend_shader_ptr pbs_box;
	cpp_blend_shader_ptr pbs_plane;

	raster_state_ptr rs_front;
	raster_state_ptr rs_back;

	surface_ptr display_surf;
	surface* pdsurf;

	font_ptr fnt;
	uint32_t num_frames;
	float accumulate_time;
	float fps;

	timer timer;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] ){
	font_sample loader;
	return loader.run();
}