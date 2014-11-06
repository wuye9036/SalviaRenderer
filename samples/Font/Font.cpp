#include <salviar/include/shader.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>
#include <salviar/include/texture.h>

#include <salviax/include/swap_chain/swap_chain.h>
#include <salviax/include/resource/font/font.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>
#include <salviax/include/resource/texture/tex_io.h>

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

// #define SALVIA_ENABLE_PIXEL_SHADER 1

struct vert
{
	vec4 pos;
};

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
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}
};

class ps_plane : public cpp_pixel_shader
{
	sampler_ptr sampler_;

public:
	ps_plane(sampler_ptr const& samp)
		: sampler_(samp)
	{
	}

	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		color_rgba32f color = tex2d(*sampler_, 0);
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

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 3 : 2000;
int const TEST_FRAME_COUNT		= 3;

class font_sample: public sample_app{
public:
	font_sample(): sample_app("Font")
	{
	}

protected:
	void on_init() override
    {
		create_devices_and_targets(512, 512, 1, pixel_format_color_bgra8, pixel_format_color_rg32f);

		viewport vp = {0, 0, 512, 512, 0.0f, 1.0f};
        data_->renderer->set_viewport(vp);
		
		planar_mesh = create_planar(
			data_->renderer.get(), 
			vec3(-1.0f, -1.0f, 0.0f), 
			vec3(1.0f, 0.0f, 0.0f), 
			vec3(0.0f, 1.0f, 0.0f),
			2, 2, true
			);
		
		pvs_plane.reset(new vs_plane());

		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_linear;
		desc.addr_mode_u = address_wrap;
		desc.addr_mode_v = address_wrap;

		plane_tex = data_->renderer->create_tex2d(512, 512, 1, pixel_format_color_rgba8);
		fnt = font::create_in_system_path("msyh.ttc", 0, 14, font::points);
		if (fnt)
		{
			wchar_t const* str = L"吞玻璃；LazyFox；1875；お帰りなさい";
			fnt->draw(to_ansi_string(str).c_str(), plane_tex->subresource(0).get(), rect<int32_t>(0, 0, 512, 512),
				color_rgba32f(0.8f, 0.8f, 1.0f, 1.0f), color_rgba32f(0.0f, 0.0f, 0.0f, 1.0f), font::antialias );
		}
		else
		{
			fnt = font::create("../../resources/font/AnglicanText.ttf", 0, 56, font::points);
			fnt->draw( "Cannot find msyh.ttc", plane_tex->subresource(0).get(), rect<int32_t>(0, 0, 512, 512),
				color_rgba32f(0.8f, 0.8f, 1.0f, 1.0f), color_rgba32f(0.0f, 0.0f, 0.0f, 1.0f), font::antialias );
		}
		plane_tex->gen_mipmap(filter_linear, true);
			
		plane_sampler = data_->renderer->create_sampler(desc, plane_tex);
        pps_plane.reset(new ps_plane(plane_sampler));

#ifdef SALVIA_ENABLE_PIXEL_SHADER
		cout << "Creating Plane Pixel Shader ..." << endl;
		psc_plane = compile( plane_ps_code, lang_pixel_shader );
#endif

		pbs_plane.reset(new ts_blend_off);

		raster_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

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

	void on_frame() override
	{
		profiling("BackBufferClearing", [this](){
			data_->renderer->clear_color(data_->color_target, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
			data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth | clear_stencil, 1.0f, 0);
		});

		vec3 camera(0.0f, 0.0f, -2.2f);

		mat44 world(mat44::identity()), view, proj, wvp;
		
		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_ortho(proj, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1000.0f);

		mat_mul(wvp, world, mat_mul(wvp, view, proj));

		data_->renderer->set_rasterizer_state(rs_back);
		pvs_plane->set_constant(_T("WorldViewProjMat"), &wvp);
		data_->renderer->set_vertex_shader(pvs_plane);
#ifdef SALVIA_ENABLE_PIXEL_SHADER
		data_->renderer->set_pixel_shader_code( psc_plane );
		data_->renderer->set_ps_sampler( "samp", plane_sampler );
#else
		data_->renderer->set_pixel_shader(pps_plane);
#endif
		data_->renderer->set_blend_shader(pbs_plane);

		profiling("Rendering", [&](){
			planar_mesh->render();
		});
	}

protected:
	mesh_ptr                planar_mesh;
	texture_ptr             plane_tex;
	sampler_ptr             plane_sampler;

	cpp_vertex_shader_ptr   pvs_plane;
	cpp_pixel_shader_ptr    pps_plane;
	shader_object_ptr       psc_plane;
	cpp_blend_shader_ptr    pbs_plane;

	raster_state_ptr        rs_back;

	font_ptr                fnt;
};

EFLIB_MAIN(argc, argv)
{
	font_sample loader;
	loader.init(argc, const_cast<std::_tchar const**>(argv));
	loader.run();

	return 0;
}
