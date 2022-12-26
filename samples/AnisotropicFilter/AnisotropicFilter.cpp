#include <salviau/include/common/sample_app.h>
#include <salviau/include/common/path.h>
#include <salviau/include/common/gui.h>
#include <salviau/include/common/window.h>

#include <salvia/shader/shader.h>
#include <salvia/shader/shader_regs.h>
#include <salvia/shader/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salvia/resource/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salvia/resource/colors.h>
#include <salvia/resource/texture.h>

#include <salviax/include/swap_chain/swap_chain.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>
#include <salviax/include/resource/texture/tex_io.h>

#include <eflib/platform/main.h>

#include <vector>
#include <cmath>

using namespace eflib;
using namespace salviar;
using namespace salviax;
using namespace salviax::resource;
using namespace salviau;

using std::shared_ptr;
using std::dynamic_pointer_cast;
using std::string;
using std::vector;
using std::cout;
using std::endl;

#define SALVIA_ENABLE_PIXEL_SHADER 1

struct vert
{
	vec4 pos;
};

float  const CYLINDER_RADIUS         = 5.0f;
size_t const CYLINDER_SEGMENTS       = 96;
float  const CYLINDER_SEG_ANGLE      = 360.0f / static_cast<float>(CYLINDER_SEGMENTS);
float  const CYLINDER_SEG_HALF_WIDTH = CYLINDER_SEGMENTS > 2 ? tanf(eflib::radians(CYLINDER_SEG_ANGLE/2.0f)) * CYLINDER_RADIUS : 2 * CYLINDER_RADIUS;

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

#if defined(EFLIB_DEBUG)
	int const BENCHMARK_TOTAL_FRAME_COUNT = 80;	
#else
	int const BENCHMARK_TOTAL_FRAME_COUNT = 1000;
#endif

class anisotropic_filter: public sample_app
{
public:
	anisotropic_filter(): sample_app("AnisotropicFilter")
	{
	}

protected:
	void on_init() override
	{
		create_devices_and_targets(data_->screen_width, data_->screen_height, 1, pixel_format_color_rgba8, pixel_format_color_rg32f);

        data_->renderer->set_viewport(data_->screen_vp);

		planar_mesh = create_planar(
			data_->renderer.get(), 
			vec3(-CYLINDER_SEG_HALF_WIDTH, 0.0f, -10.0f), 
			vec3(CYLINDER_SEG_HALF_WIDTH, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			2, 50, true
			);
		
		pvs_plane.reset(new vs_plane());

		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_anisotropic;
		desc.max_anisotropy = 16;

		filter_params = decltype(filter_params) {
			// MIP FILTER, FILTER QUALITY
			{ filter_linear, mip_lo_quality, 0, "Mipmap, Low Quality" },
			{ filter_linear, mip_mi_quality, 0, "Mipmap, Medium Quality" },
			{ filter_linear, mip_hi_quality, 0,  "Mipmap, High Quality" },
			{ filter_anisotropic, mip_mi_quality, 2, "AF 2X"},
			{ filter_anisotropic, mip_mi_quality, 4, "AF 4X" },
			{ filter_anisotropic, mip_mi_quality, 8, "AF 8X" },
			{ filter_anisotropic, mip_mi_quality, 16, "AF 16X" }
		};


		// auto plane_tex_path = find_path(_EFLIB_T("texture_and_blending/chessboard.png"));
		auto plane_tex_path = find_path(_EFLIB_T("font/font_enu.png"));
		if(plane_tex_path.empty())
		{
			throw "Plane texture loading failed.";
		}

		plane_tex = load_texture(data_->renderer.get(), plane_tex_path, salviar::pixel_format_color_rgba8);
		plane_tex->gen_mipmap(filter_linear, true);

		// plane_tex->subresource(2)->fill_texels( color_rgba32f(0.0f, 0.0f, 1.0f, 0.0f) );
		// plane_tex->subresource(3)->fill_texels( color_rgba32f(1.0f, 0.0f, 0.0f, 0.0f) );
		// plane_tex->subresource(4)->fill_texels( color_rgba32f(0.0f, 1.0f, 0.0f, 0.0f) );
		// plane_tex->subresource(5)->fill_texels( color_rgba32f(0.0f, 0.0f, 1.0f, 0.0f) );

		plane_sampler = data_->renderer->create_sampler(desc, plane_tex);
        pps_plane.reset(new ps_plane(plane_sampler));

#ifdef SALVIA_ENABLE_PIXEL_SHADER
		cout << "Creating Plane Pixel Shader ..." << endl;
		psc_plane = compile( plane_ps_code, lang_pixel_shader );
#endif

		pbs_plane.reset(new ts_blend_off);

		raster_desc rs_desc;
		rs_desc.cm = cull_front;
		rs_front.reset(new raster_state(rs_desc));
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));
	}

	void on_frame() override
	{
		switch(data_->mode)
		{
		case salviau::app_modes::benchmark:
			data_->quiting = (data_->frame_count == BENCHMARK_TOTAL_FRAME_COUNT);
			break;
		case salviau::app_modes::test:
			data_->quiting = (data_->frame_count == filter_params.size());
			break;
		}

		if(data_->quiting)
		{
			return;
		}

		profiling("BackBufferClearing", [this](){
			data_->renderer->clear_color(data_->color_target, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
			data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth | clear_stencil, 1.0f, 0);
		});

		mat44 world(mat44::identity()), view, proj, wvp;
		
		mat_lookat(view, vec3(0.0f, 0.0f, -10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), data_->screen_aspect_ratio, 0.1f, 100.0f);

		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;

		size_t i_param = 0;

		switch(data_->mode)
		{
		case salviau::app_modes::benchmark:
		case salviau::app_modes::test:
			i_param = data_->frame_count % filter_params.size();
			break;
		case salviau::app_modes::interactive:
			break;
		case salviau::app_modes::replay:
			i_param = static_cast<size_t>(data_->total_elapsed_sec / 3.0) % filter_params.size();
			break;
		}

		std::string window_title;
		std::tie(desc.mip_filter, desc.mip_qual, desc.max_anisotropy, window_title) = filter_params[i_param];

		if(data_->gui) { data_->gui->main_window()->set_title(window_title); }

        plane_sampler = data_->renderer->create_sampler(desc, plane_tex);

		profiling("Rendering", [&](){
			data_->renderer->set_rasterizer_state(rs_back);

			for(size_t i = 0; i < CYLINDER_SEGMENTS; ++i)
			{
				// if (i != 0) continue;
				mat44 rot_mat;
				mat_rotZ(rot_mat, i * CYLINDER_SEG_ANGLE);
				mat_translate(world, 0.0, -CYLINDER_RADIUS, 0.0f);
				mat_mul(world, world, rot_mat);

				mat_mul(wvp, world, mat_mul(wvp, view, proj));

				pvs_plane->set_constant(_T("WorldViewProjMat"), &wvp);
				data_->renderer->set_vertex_shader(pvs_plane);
		#ifdef SALVIA_ENABLE_PIXEL_SHADER
				data_->renderer->set_pixel_shader_code( psc_plane );
				data_->renderer->set_ps_sampler("samp", plane_sampler);
		#else
				data_->renderer->set_pixel_shader(pps_plane);
		#endif
				data_->renderer->set_blend_shader(pbs_plane);
		
				planar_mesh->render();
			}


		});
	}

protected:
	texture_ptr             sm_tex;
    
	mesh_ptr                planar_mesh;
	texture_ptr             plane_tex;
	sampler_ptr             plane_sampler;

	cpp_vertex_shader_ptr   pvs_plane;
	cpp_pixel_shader_ptr    pps_plane;
	shader_object_ptr       psc_plane;

	cpp_blend_shader_ptr    pbs_plane;

	raster_state_ptr        rs_front;
	raster_state_ptr        rs_back;

	std::vector<std::tuple<filter_type, mip_quality, int/*aniso*/, std::string/*name*/>> filter_params;
};

EFLIB_MAIN(argc, argv)
{
    anisotropic_filter loader;
	loader.init( argc, const_cast<std::_tchar const**>(argv) );
	loader.run();

	return 0;
}
