#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/presenter_dev.h>
#include <salviar/include/shader.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/terrain/gen_terrain.h>

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
// #define SALVIA_PIXEL_SHADER_ENABLED

int const TERRAIN_BLOCKS	= 32;
int const TERRAIN_BLOCK_SIZE= 32;
int const TERRAIN_SIZE		= TERRAIN_BLOCKS * TERRAIN_BLOCK_SIZE;

char const* vs_code =
"float4x4 wvpMatrix; \r\n"
"float2   terrainOffset; \r\n"
"float2	  terrainScale; \r\n"
"sampler  terrainSamp; \r\n"
"struct VSIn{ \r\n"
"	float4 pos: POSITION; \r\n"
"	float4 uv: TEXCOORD0; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: sv_position; \r\n"
"	float  displacement: TEXCOORD0; \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
"	float2 terrainUV = terrainOffset + in.uv.xy * terrainScale; \r\n"
"	float displacement = tex2Dlod( terrainSamp, float4(terrainUV, 0.0f, 0.0f) ).x; \r\n"
"	float4 displaced_pos = float4( in.pos.xyz + float3(0.0f, displacement * 20.0f, 0.0f), 1.0f ); \r\n"
"	out.pos = mul(displaced_pos, wvpMatrix); \r\n"
"	out.displacement = displacement; \r\n"
"	return out; \r\n"
"} \r\n"
;

char const* ps_code =
"float4 color; \r\n"
"float4 ps_main( float c: TEXCOORD0 ): COLOR \r\n"
"{ \r\n"
"	return float4(c, c, c, 1.0f); \r\n"
"} \r\n"
;

class ps : public pixel_shader
{
public:
	vec4 color;
	ps()
	{
		declare_constant(_T("Color"),   color );
		
		bind_semantic("TEXCOORD", 0, 0);
	}
	bool shader_prog(const vs_output& in, ps_output& out)
	{
		float height = in.attribute(0)[0];

		vec4 colors[] = 
		{
			vec4(0.0f, 0.0f, 0.5f, 1.0f),
			vec4(0.7f, 0.6f, 0.0f, 1.0f),
			vec4(0.45f, 0.38f, 0.26f, 1.0f),
			vec4(0.0f, 0.7f, 0.8f, 1.0f),
			vec4(0.9f, 0.9f, 1.0f, 1.0f),
			vec4(0.9f, 0.9f, 1.0f, 1.0f) // sentinel
		};

		float boundary_points[] = {0.0f, 0.62f, 0.75f, 0.88f, 1.0f, 1.0f};

		int lower_bound = -1;
		for(int i = 0; i < 5; ++i)
		{
			if( height < boundary_points[i] ){
				break;
			}
			lower_bound = i;
		}
		
		if(lower_bound == -1)
		{
			out.color[0] = colors[0];
		}
		else
		{
			float lower_value = boundary_points[lower_bound];
			float interval = boundary_points[lower_bound+1]-lower_value;
			out.color[0] = lerp( colors[lower_bound], colors[lower_bound+1], (height-lower_value)/interval );
		}

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

class vertex_texture_fetch: public quick_app{
public:
	vertex_texture_fetch(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		cout << "Creating window and device ..." << endl;

		string title( "Sample: Vertex Texture Fetch" );
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

		cout << "Generate Random Terrain..." << endl;
		vector<float> field;
		salviax::resource::make_terrain_plasma(field, TERRAIN_SIZE, 0.5f);
		salviax::resource::filter_terrain(field, TERRAIN_SIZE, 0.15f);

		terrain_tex = salviax::resource::make_terrain_texture(hsr.get(), field, TERRAIN_SIZE);
		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_linear;
			desc.addr_mode_u = address_mirror;
			desc.addr_mode_v = address_mirror;
			desc.addr_mode_w = address_mirror;

			terrain_samp = hsr->create_sampler( desc );
			terrain_samp->set_texture( terrain_tex.get() );
		}

		cout << "Compiling vertex shader ... " << endl;
		vsc = compile( vs_code, lang_vertex_shader );
		hsr->set_vertex_shader_code( vsc );

#ifdef SALVIA_PIXEL_SHADER_ENABLED
		cout << "Compiling pixel shader ... " << endl;
		psc = compile( ps_code, lang_pixel_shader );
		hsr->set_pixel_shader_code(psc);
#endif

		plane = create_planar(
			hsr.get(), 
			vec3(-TERRAIN_BLOCK_SIZE/2.0f, 0.0f, -TERRAIN_BLOCK_SIZE/2.0f), 
			vec3(0.5f, 0.0f, 0.0f), 
			vec3(0.0f, 0.0f, 0.5f),
			TERRAIN_BLOCK_SIZE*2, TERRAIN_BLOCK_SIZE*2, false
			);

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

		vec3 camera( 0.0f, 32.0f, -7.0f);
		vec4 camera_pos = vec4( camera, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;
		mat_identity( world );
		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);

		static float offset_x = 0.0f;
		static float offset_y = 0.0f;
		static float scale = 1.0f / 32;

		offset_x += (0.006f * elapsed_time);
		offset_y += (0.0088f * elapsed_time);

		hsr->set_pixel_shader(pps);
		hsr->set_blend_shader(pbs);

		hsr->set_rasterizer_state(rs_back);

		vec2 terrain_scale(scale, scale);
		vec2 terrain_offset(offset_x, offset_y);

		hsr->set_vertex_shader_code( vsc );

		mat_mul(wvp, world, mat_mul(wvp, view, proj));
		hsr->set_vs_variable( "wvpMatrix", &wvp );

		hsr->set_vs_variable( "terrainScale", &terrain_scale );
		hsr->set_vs_variable( "terrainOffset", &terrain_offset );
		hsr->set_vs_sampler( "terrainSamp", terrain_samp);
		
#ifdef SALVIA_PIXEL_SHADER_ENABLED
		hsr->set_pixel_shader_code(psc);
		hsr->set_ps_variable( "color", &color );
#endif
		vec4 color = vec4( 0.3f, 0.7f, 0.3f, 1.0f );

		pps->set_constant( _T("Color"), &color );
		plane->render();

		if (hsr->get_framebuffer()->get_num_samples() > 1){
			hsr->get_framebuffer()->get_render_target(render_target_color, 0)->resolve(*display_surf);
		}

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	h_device present_dev;
	h_renderer hsr;

	h_mesh			plane;
	texture_ptr		terrain_tex;
	h_sampler		terrain_samp;

	shared_ptr<shader_object> vsc;
	shared_ptr<shader_object> psc;

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
	vertex_texture_fetch loader;
	return loader.run();
}