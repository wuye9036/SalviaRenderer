#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/shader.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/swap_chain/swap_chain.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/terrain/gen_terrain.h>

#include <salviau/include/common/timer.h>
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

class ps : public cpp_pixel_shader
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
	virtual cpp_pixel_shader_ptr create_clone()
	{
		return cpp_pixel_shader_ptr(new ps(*this));
	}
	virtual void destroy_clone(cpp_pixel_shader_ptr& ps_clone)
	{
		ps_clone.reset();
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

		raster_desc rs_desc;
		rs_desc.cm = cull_none;
		rs_back.reset(new raster_state(rs_desc));

		cout << "Generate Random Terrain..." << endl;
		vector<float> field;
		salviax::resource::make_terrain_plasma(field, TERRAIN_SIZE, 0.5f);
		salviax::resource::filter_terrain(field, TERRAIN_SIZE, 0.15f);

		terrain_tex = salviax::resource::make_terrain_texture(renderer_.get(), field, TERRAIN_SIZE);
		{
			sampler_desc desc;
			desc.min_filter = filter_linear;
			desc.mag_filter = filter_linear;
			desc.mip_filter = filter_linear;
			desc.addr_mode_u = address_mirror;
			desc.addr_mode_v = address_mirror;
			desc.addr_mode_w = address_mirror;

			terrain_samp = renderer_->create_sampler( desc );
			terrain_samp->set_texture( terrain_tex.get() );
		}

		cout << "Compiling vertex shader ... " << endl;
		vsc = compile( vs_code, lang_vertex_shader );
		renderer_->set_vertex_shader_code( vsc );

#ifdef SALVIA_PIXEL_SHADER_ENABLED
		cout << "Compiling pixel shader ... " << endl;
		psc = compile( ps_code, lang_pixel_shader );
		renderer_->set_pixel_shader_code(psc);
#endif

		plane = create_planar(
			renderer_.get(), 
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
		renderer_->clear_depth_stencil(ds_surface_, 1.0f, 0);

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

		renderer_->set_pixel_shader(pps);
		renderer_->set_blend_shader(pbs);

		renderer_->set_rasterizer_state(rs_back);

		vec2 terrain_scale(scale, scale);
		vec2 terrain_offset(offset_x, offset_y);

		renderer_->set_vertex_shader_code( vsc );

		mat_mul(wvp, world, mat_mul(wvp, view, proj));
		renderer_->set_vs_variable( "wvpMatrix", &wvp );

		renderer_->set_vs_variable( "terrainScale", &terrain_scale );
		renderer_->set_vs_variable( "terrainOffset", &terrain_offset );
		renderer_->set_vs_sampler( "terrainSamp", terrain_samp);
		
#ifdef SALVIA_PIXEL_SHADER_ENABLED
		renderer_->set_pixel_shader_code(psc);
		renderer_->set_ps_variable( "color", &color );
#endif
		vec4 color = vec4( 0.3f, 0.7f, 0.3f, 1.0f );

		pps->set_constant( _T("Color"), &color );
		plane->render();

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	swap_chain_ptr          swap_chain_;
	renderer_ptr            renderer_;

    surface_ptr             ds_surface_;
    surface_ptr             color_surface_;

	mesh_ptr			    plane;
	texture_ptr		        terrain_tex;
	sampler_ptr		        terrain_samp;

	shader_object_ptr       vsc;
	shader_object_ptr       psc;

	cpp_vertex_shader_ptr	pvs;
	cpp_pixel_shader_ptr	pps;
	cpp_blend_shader_ptr	pbs;

	raster_state_ptr        rs_back;

	uint32_t                num_frames;
	float                   accumulate_time;
	float                   fps;

	timer                   timer;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] ){
	vertex_texture_fetch loader;
	return loader.run();
}