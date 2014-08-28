#include <tchar.h>

#include <salviau/include/win/win_application.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/swap_chain/swap_chain.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>

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

char const* vs_code =
"float4x4 wvpMatrix; \r\n"
"struct VSIn{ \r\n"
"	float4 pos: POSITION; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: sv_position; \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
"	out.pos = mul(in.pos, wvpMatrix); \r\n"
"	return out; \r\n"
"} \r\n"
;

char const* ps_code =
"float4 color; \r\n"
"float4 ps_main(): COLOR \r\n"
"{ \r\n"
"	return color; \r\n"
"} \r\n"
;

class ps : public cpp_pixel_shader
{
public:
	vec4 color;
	ps()
	{
		declare_constant(_T("Color"),   color );
	}
	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		out.color[0] = color;
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

class triangles: public quick_app{
public:
	triangles(): quick_app( create_win_gui() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		cout << "Creating window and device ..." << endl;

		string title( "Sample: Triangles" );
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
            )->subresource(0);
        renderer_->set_render_targets(1, &color_surface_, ds_surface_);

        viewport vp;
        vp.w = static_cast<float>(render_params.backbuffer_width);
        vp.h = static_cast<float>(render_params.backbuffer_height);
        vp.x = 0;
        vp.y = 0;
        vp.minz = 0.0f;
        vp.maxz = 1.0f;
        renderer_->set_viewport(vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_none;
		rs_back.reset(new raster_state(rs_desc));

		cout << "Compiling vertex shader ... " << endl;
		vsc = compile( vs_code, lang_vertex_shader );
		renderer_->set_vertex_shader_code( vsc );

		cout << "Compiling pixel shader ... " << endl;
		psc = compile( ps_code, lang_pixel_shader );
#ifdef SALVIA_PIXEL_SHADER_ENABLED
		renderer_->set_pixel_shader_code(psc);
#endif
		mesh_ptr pmesh;

		pmesh = create_planar(
			renderer_.get(), 
			vec3(-30.0f, -1.0f, -30.0f), 
			vec3(15.0f, 0.0f, 1.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			4, 60, false
			);
		meshes.push_back( pmesh );

		pmesh = create_planar(
			renderer_.get(), 
			vec3(-5.0f, -5.0f, -30.0f), 
			vec3(0.0f, 4.0f, 0.0f), 
			vec3(0.0f, 0.0f, 1.0f),
			4, 60, false
			);

		meshes.push_back( pmesh );

		pmesh = create_box( renderer_.get() );
		meshes.push_back( pmesh );

		num_frames = 0;
		accumulate_time = 0;
		fps = 0;

		pps.reset( new ps() );
		pbs.reset( new bs() );
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

		static float t = 17.08067f;
		// t += elapsed_time / 50000.0f;
		float r = 2.0f * ( 1 + t );
		float x = r * cos(t);
		float y = r * sin(t);

		vec3 camera( x, 8.0f, y);
		vec4 camera_pos = vec4( camera, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;
		mat_scale( world, 30.0f, 30.0f, 30.0f );
		mat_lookat(view, camera, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);

		static float ypos = 40.0f;
		ypos -= elapsed_time;
		if ( ypos < 1.0f ){
			ypos = 40.0f;
		}

		renderer_->set_pixel_shader(pps);
		renderer_->set_blend_shader(pbs);

		for(float i = 0 ; i < 1 ; i ++)
		{
			// mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			renderer_->set_rasterizer_state(rs_back);

			// C++ vertex shader and SASL vertex shader are all available.
			renderer_->set_vertex_shader_code( vsc );
			renderer_->set_vs_variable( "wvpMatrix", &wvp );

#ifdef SALVIA_PIXEL_SHADER_ENABLED
			renderer_->set_pixel_shader_code(psc);
#endif
			vec4 color[3];
			color[0] = vec4( 0.3f, 0.7f, 0.3f, 1.0f );
			color[1] = vec4( 0.3f, 0.3f, 0.7f, 1.0f );
			color[2] = vec4( 0.7f, 0.3f, 0.3f, 1.0f );

			for( size_t i_mesh = 0; i_mesh < meshes.size(); ++i_mesh ){
				mesh_ptr cur_mesh = meshes[i_mesh];
				renderer_->set_ps_variable( "color", &color[i_mesh] );
				pps->set_constant( _T("Color"), &color[i_mesh] );
				cur_mesh->render();
			}
		}

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	swap_chain_ptr          swap_chain_;
	renderer_ptr            renderer_;
    
    surface_ptr             ds_surface_;
    surface_ptr             color_surface_;

	vector<mesh_ptr>		meshes;
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
	triangles loader;
	return loader.run();
}