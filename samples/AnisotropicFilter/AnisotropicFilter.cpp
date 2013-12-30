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
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>
#include <salviax/include/resource/texture/tex_io.h>
#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

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

#define SALVIA_ENABLE_PIXEL_SHADER 1

struct vert
{
	vec4 pos;
};

class vs_cone : public cpp_vertex_shader 
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
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
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

class vs_plane : public cpp_vertex_shader
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
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}

};

class ps_plane : public cpp_pixel_shader
{
	salviar::sampler_ptr samp_;

public:
	ps_plane(sampler_ptr const& samp)
		: samp_(samp)
	{
	}

	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f color = tex2d(*samp_, 0);
		color.a = 1;
		out.color[0] = color.get_vec4();

		return true;
	}

    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
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
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
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
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
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
		
		planar_mesh = create_planar(
			renderer_.get(), 
			vec3(-50.0f, 0.0f, -50.0f), 
			vec3(1.0f, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			100, 100, true
			);
		
		pvs_plane.reset(new vs_plane());

		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_anisotropic;
			desc.max_anisotropy = 16;

			plane_tex = load_texture(renderer_.get() , _T("../../resources/chessboard.png") , salviar::pixel_format_color_rgba8);
			plane_tex->gen_mipmap(filter_linear, true);
			
			plane_sampler = renderer_->create_sampler(desc, plane_tex);
            pps_plane.reset(new ps_plane(plane_sampler));

#ifdef SALVIA_ENABLE_PIXEL_SHADER
			cout << "Creating Plane Pixel Shader ..." << endl;
			psc_plane = compile( plane_ps_code, lang_pixel_shader );
#endif
		}

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

	void on_draw()
    {
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

        renderer_->clear_color(color_surface_, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		renderer_->clear_depth_stencil(ds_surface_, clear_depth | clear_stencil, 1.0f, 0);

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
        plane_sampler = renderer_->create_sampler(desc, plane_tex);

		for(float i = 0 ; i < 1 ; i ++)
		{	
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			renderer_->set_rasterizer_state(rs_back);
			pvs_plane->set_constant(_T("WorldViewProjMat"), &wvp);
			renderer_->set_vertex_shader(pvs_plane);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
			renderer_->set_pixel_shader_code( psc_plane );
			renderer_->set_ps_sampler( "samp", plane_sampler );
#else
			renderer_->set_pixel_shader(pps_plane);
#endif
			renderer_->set_blend_shader(pbs_plane);
			planar_mesh->render();
		}

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	swap_chain_ptr          swap_chain_;
	renderer_ptr            renderer_;
	texture_ptr             sm_tex;
    
    surface_ptr             ds_surface_;
    surface_ptr             color_surface_;

	mesh_ptr                planar_mesh;
	texture_ptr             plane_tex;
	sampler_ptr             plane_sampler;

	cpp_vertex_shader_ptr   pvs_plane;
	cpp_pixel_shader_ptr    pps_plane;
	shader_object_ptr       psc_plane;

	cpp_blend_shader_ptr    pbs_plane;

	raster_state_ptr        rs_front;
	raster_state_ptr        rs_back;

	uint32_t                num_frames;
	float                   accumulate_time;
	float                   fps;

	timer                   timer;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] ){
	anisotropic_filter loader;
	return loader.run();
}