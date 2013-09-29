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

#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>
#include <salviax/include/resource/mesh/sa/material.h>
#include <salviax/include/resource/texture/freeimage/tex_io_freeimage.h>

#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#include <eflib/include/diagnostics/profiler.h>

#include <vector>

#if defined(EFLIB_WINDOWS)
#define NOMINMAX
#include <Windows.h>
#endif

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
using boost::dynamic_pointer_cast;

using std::string;
using std::vector;
using std::cout;
using std::endl;

#define SASL_VERTEX_SHADER_ENABLED

char const* benchmark_vs_code =
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

class benchmark_vs : public cpp_vertex_shader
{
	mat44 wvp;
	vec4 light_pos, eye_pos;
public:
	benchmark_vs():wvp(mat44::identity()){
		declare_constant(_T("wvpMatrix"), wvp);
		declare_constant(_T("lightPos"),  light_pos);
		declare_constant(_T("eyePos"),    eye_pos);

		bind_semantic("POSITION", 0, 0);
		bind_semantic("TEXCOORD", 0, 1);
		bind_semantic("NORMAL",   0, 2);
	}

	benchmark_vs(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attribute(0);
		transform(out.position(), pos, wvp);
		out.attribute(0) = in.attribute(1);
		out.attribute(1) = in.attribute(2);
		out.attribute(2) = light_pos - pos;
		out.attribute(3) = eye_pos - pos;
	}

	uint32_t num_output_attributes() const
	{
		return 4;
	}

	uint32_t output_attribute_modifiers(uint32_t) const
	{
		return salviar::vs_output::am_linear;
	}
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}

};

class benchmark_ps : public cpp_pixel_shader
{
	salviar::sampler_ptr sampler_;
	salviar::texture_ptr tex_;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	int shininess;
public:
	void set_texture( salviar::texture_ptr tex ){
		tex_ = tex;
		sampler_->set_texture(tex_.get());
	}

	benchmark_ps()
	{
		declare_constant(_T("Ambient"),   ambient );
		declare_constant(_T("Diffuse"),   diffuse );
		declare_constant(_T("Specular"),  specular );
		declare_constant(_T("Shininess"), shininess );

		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_point;
		desc.addr_mode_u = address_wrap;
		desc.addr_mode_v = address_wrap;
		desc.addr_mode_w = address_wrap;
		sampler_.reset(new sampler(desc));
	}

	bool shader_prog(const vs_output& in, ps_output& out)
	{
		vec4 diff_color = vec4(1.0f, 1.0f, 1.0f, 1.0f); // diffuse;

		if( tex_ )
		{
			diff_color = tex2d(*sampler_, 0).get_vec4();
		}

		vec3 norm( normalize3( in.attribute(1).xyz() ) );
		vec3 light_dir( normalize3( in.attribute(2).xyz() ) );
		vec3 eye_dir( normalize3( in.attribute(3).xyz() ) );

		float illum_diffuse  = clamp(dot_prod3(light_dir, norm), 0.0f, 1.0f);
		float illum_specular = clamp(dot_prod3(reflect3(light_dir, norm), eye_dir), 0.0f, 1.0f);

		out.color[0] = ambient * 0.01f + diff_color * illum_diffuse + specular * illum_specular;
		out.color[0] = diff_color * illum_diffuse;
		out.color[0][3] = 1.0f;

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

class benchmark
{
public:
	benchmark()
	{
		prof.start("Benchmark", 0);
		initialize();
	}

	~benchmark()
	{
		prof.end("Benchmark");
		prof.merge_items();
		print_profiler(&prof, 3);
	}

	void initialize()
	{
		color_format_ = pixel_format_color_bgra8;
		height_ = 512;
		width_ = 512;
		sample_count_ = 1;

		renderer_ = create_benchmark_renderer();

        color_surface_ = renderer_->create_tex2d(width_, height_, sample_count_, color_format_)->get_surface(0);
        ds_surface_ = renderer_->create_tex2d(width_, height_, sample_count_, pixel_format_color_rg32f)->get_surface(0);
        if(sample_count_ == 1)
        {
            resolved_color_surface_ = color_surface_;
        }
        else
        {
            resolved_color_surface_ = renderer_->create_tex2d(width_, height_, 1, color_format_)->get_surface(0);
        }
        renderer_->set_render_targets(1, &color_surface_, ds_surface_);
        
        viewport vp;
        vp.w = static_cast<float>(width_);
        vp.h = static_cast<float>(height_);
        vp.x = 0;
        vp.y = 0;
        vp.minz = 0.0f;
        vp.maxz = 1.0f;
        renderer_->set_viewport(vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

#ifdef SASL_VERTEX_SHADER_ENABLED
		cout << "Compiling vertex shader ... " << endl;
		prof.start("Vertex Shader Compiling", 0);
		benchmark_vs = compile( benchmark_vs_code, lang_vertex_shader );
		prof.end("Vertex Shader Compiling");
#endif

		cout << "Loading mesh ... " << endl;
		prof.start("Mesh Loading", 0);
		benchmark_mesh = create_mesh_from_obj(renderer_.get(), "../../resources/models/sponza/sponza.obj", false);
		prof.end("Mesh Loading");
		cout << "Loading pixel and blend shader... " << endl;

		cpp_vs.reset( new ::benchmark_vs() );
		cpp_ps.reset( new ::benchmark_ps() );
		cpp_bs.reset( new ::bs() );
	}
	/** @} */

	void save_frame(std::string const& file_name)
	{
		cout << "Save" << endl;
		prof.start("Saving", 0);
        if (color_surface_ != resolved_color_surface_)
		{
			color_surface_->resolve(*resolved_color_surface_);
		}
		texture_io_fi::instance().save(*resolved_color_surface_, to_tstring(file_name), pixel_format_color_bgra8);
		prof.end("Saving");
	}

	void render()
	{
		prof.start("Back buffer Clearing", 0);
        renderer_->clear_color(color_surface_, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		renderer_->clear_depth_stencil(ds_surface_, 1.0f, 0);
		prof.end("Back buffer Clearing");

		prof.start("Set rendering parameters", 0);

		vec3 camera(-36.0f, 8.0f, 0.0f);
		vec4 camera_pos = vec4( camera, 1.0f );
		
		mat44 world(mat44::identity()), view, proj, wvp;
		mat_lookat(view, camera, vec3(40.0f, 15.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);
		
		vec4 lightPos( 0.0f, 40.0f, 0.0f, 1.0f );

		renderer_->set_pixel_shader(cpp_ps);
		renderer_->set_blend_shader(cpp_bs);

		mat_translate(world , -0.5f, 0, -0.5f);
		mat_mul(wvp, world, mat_mul(wvp, view, proj));

		renderer_->set_rasterizer_state(rs_back);

		// C++ vertex shader and SASL vertex shader are all available.
#ifdef SASL_VERTEX_SHADER_ENABLED
		renderer_->set_vertex_shader_code(benchmark_vs);
#else
		cpp_vs->set_constant( _T("wvpMatrix"), &wvp );
		cpp_vs->set_constant( _T("eyePos"), &camera_pos );
		cpp_vs->set_constant( _T("lightPos"), &lightPos );
		renderer_->set_vertex_shader(cpp_vs);
#endif
		renderer_->set_vs_variable( "wvpMatrix", &wvp );
		
		renderer_->set_vs_variable( "eyePos", &camera_pos );
		renderer_->set_vs_variable( "lightPos", &lightPos );
		
		prof.end("Set rendering parameters");

		prof.start("Rendering", 0);
		for( size_t i_mesh = 0; i_mesh < benchmark_mesh.size(); ++i_mesh )
		{
			mesh_ptr cur_mesh = benchmark_mesh[i_mesh];

			shared_ptr<obj_material> mtl
				= dynamic_pointer_cast<obj_material>( cur_mesh->get_attached() );

			cpp_ps->set_constant( _T("Ambient"),  &mtl->ambient );
			cpp_ps->set_constant( _T("Diffuse"),  &mtl->diffuse );
			cpp_ps->set_constant( _T("Specular"), &mtl->specular );
			cpp_ps->set_constant( _T("Shininess"),&mtl->ambient );
			dynamic_pointer_cast<benchmark_ps>(cpp_ps)->set_texture(mtl->tex);
			
			cur_mesh->render();
		}
		prof.end("Rendering");
	}

protected:
	/** Properties @{ */
	renderer_ptr			renderer_;
	vector<mesh_ptr>		benchmark_mesh;
	shader_object_ptr 	    benchmark_vs;

    pixel_format            color_format_;
    size_t                  width_;
    size_t                  height_;
    uint32_t                sample_count_;
    surface_ptr             color_surface_;
    surface_ptr             resolved_color_surface_;
    surface_ptr             ds_surface_;

	cpp_vertex_shader_ptr	cpp_vs;
	cpp_pixel_shader_ptr	cpp_ps;
	cpp_blend_shader_ptr	cpp_bs;

	raster_state_ptr	    rs_back;
	profiler			    prof;
};

#if defined(EFLIB_DEBUG)
static size_t const RENDER_FRAME_COUNT = 1;
#else
static size_t const RENDER_FRAME_COUNT = 60;
#endif

int main( int /*argc*/, TCHAR* /*argv*/[] )
{
#if defined(EFLIB_WINDOWS)
	HANDLE process_handle = GetCurrentProcess();
	SetPriorityClass(process_handle, HIGH_PRIORITY_CLASS);
#endif

	{
		benchmark bm;
		for(size_t i_frame = 1; i_frame <= RENDER_FRAME_COUNT; ++i_frame)
		{
			cout << "Render Frame #" << i_frame << "/" << RENDER_FRAME_COUNT << endl;
			bm.render();
		}
		bm.save_frame("sponza_frame.png");
	}
	system("pause");
	return 0;
}