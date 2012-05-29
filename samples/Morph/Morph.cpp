#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/presenter_dev.h>
#include <salviar/include/shader.h>
#include <salviar/include/shader_code.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/resource/mesh/sa/material.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_collada.h>

#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#include <eflib/include/platform/dl_loader.h>

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

char const* morph_vs_code =
"float4x4 wvpMatrix; \r\n"
"float4   eyePos; \r\n"
"float4	  lightPos; \r\n"
"float	  blendWeight;\r\n"
" \r\n"
"struct VSIn{ \r\n"
"	float3 pos0: POSITION0; \r\n"
"	float3 norm0: NORMAL0; \r\n"
"	float3 pos1: POSITION1; \r\n"
"	float3 norm1: NORMAL1; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: sv_position; \r\n"
"	float4 norm: TEXCOORD0; \r\n"
"	float4 lightDir: TEXCOORD1; \r\n"
"	float4 eyeDir: TEXCOORD2; \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
"	float3 morphed_pos = in.pos0 + (in.pos1-in.pos0) * blendWeight.xxx; \r\n"
"	float4 morphed_pos_v4f32 = float4(morphed_pos, 1.0f); \r\n"
//"	float3 projected_n0 = in.norm0 / in.norm0.zzz; \r\n"
//"	float3 projected_n1 = in.norm1 / in.norm1.zzz; \r\n"
//"	float3 morphed_projected_n = projected_n0 + (projected_n1-projected_n0) * blendWeight.xxx; \r\n"
"	out.pos = mul( morphed_pos_v4f32, wvpMatrix ); \r\n"
//"	float3 morphed_n = morphed_projected_n / length(morphed_projected_n).xxx; \r\n"
"	out.norm = float4( in.norm0+(in.norm1-in.norm0)*blendWeight.xxx, 0.0f );\r\n"
"	out.lightDir = lightPos-morphed_pos_v4f32; \r\n"
"	out.eyeDir = eyePos-morphed_pos_v4f32; \r\n"
"	return out; \r\n"
"} \r\n"
;

FILE* f = NULL;

class morph_vs : public vertex_shader
{
	mat44 wvp;
	vec4 light_pos, eye_pos;
	vector<mat44> invMatrices;
	vector<mat44> boneMatrices;
public:
	morph_vs():wvp(mat44::identity()){
		declare_constant(_T("wvpMatrix"), wvp);
		declare_constant(_T("lightPos"), light_pos);
		declare_constant(_T("eyePos"), eye_pos);
		declare_constant(_T("invMatrices"), invMatrices);
		declare_constant(_T("boneMatrices"), boneMatrices);

		bind_semantic( "POSITION", 0, 0 );
		bind_semantic( "NORMAL", 0, 1 );
		bind_semantic( "TEXCOORD", 0, 2 );
		bind_semantic( "BLEND_INDICES", 0, 3);
		bind_semantic( "BLEND_WEIGHTS", 0, 4);
	}

	morph_vs(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attributes[0];
		vec4 nor = in.attributes[1];

		out.position = out.attributes[0] = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		pos.w(1.0f);
		nor.w(0.0f);
		/*out.position = pos;
		out.attributes[0] = nor;*/
		
		for(int i = 0; i < 4; ++i)
		{
			union {float f; int i;} f2i;
			f2i.f = in.attributes[3][i];
			float w = in.attributes[4][i];
			int boneIndex = f2i.i;
			if(boneIndex == -1){break;}
			// fprintf(f, "%2d ", boneIndex);
			vec4 skin_pos;
			vec4 skin_nor;
			transform(skin_pos, invMatrices[boneIndex], pos);
			transform(skin_pos, boneMatrices[boneIndex], skin_pos);
			transform(skin_nor, invMatrices[boneIndex], nor);
			transform(skin_nor, boneMatrices[boneIndex], skin_nor);
			out.position += (skin_pos*w);
			out.attributes[0] += (skin_nor*w);
		}
		
#ifdef EFLIB_DEBUG
		fprintf(f, "(%8.4f %8.4f %8.4f %8.4f)", out.position[0], out.position[1], out.position[2], out.position[3]);
#endif
		transform(out.position, out.position, wvp);
#ifdef EFLIB_DEBUG
		fprintf(f, "\n");
#endif

		// out.attributes[0] = in.attributes[1];
		out.attributes[1] = in.attributes[2];
		out.attributes[2] = light_pos - pos;
		out.attributes[3] = eye_pos - pos;
	}

	uint32_t num_output_attributes() const{
		return 4;
	}

	uint32_t output_attribute_modifiers(uint32_t) const{
		return salviar::vs_output::am_linear;
	}
};

class morph_ps : public pixel_shader
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

	morph_ps()
	{
		declare_constant(_T("Ambient"),   ambient );
		declare_constant(_T("Diffuse"),   diffuse );
		declare_constant(_T("Specular"),  specular );
		declare_constant(_T("Shininess"), shininess );

		sampler_desc desc;
		desc.min_filter = filter_linear;
		desc.mag_filter = filter_linear;
		desc.mip_filter = filter_linear;
		desc.addr_mode_u = address_wrap;
		desc.addr_mode_v = address_wrap;
		desc.addr_mode_w = address_wrap;
		sampler_.reset(new sampler(desc));
	}

	bool shader_prog(const vs_output& in, ps_output& out)
	{
		vec4 ambi_color(0.22f, 0.20f, 0.09f, 1.0f);
		vec4 diff_color(0.75f, 0.75f, 0.25f, 1.0f);
		vec4 spec_color(2.0f, 1.7f, 0.0f, 1.0f);

		if( tex_ ){
			diff_color = tex2d(*sampler_, 0).get_vec4();
		}

		vec3 norm( normalize3( in.attributes[0].xyz() ) );
		vec3 light_dir( normalize3( in.attributes[1].xyz() ) );
		vec3 eye_dir( normalize3( in.attributes[2].xyz() ) );

		float illum_diffuse = clamp( dot_prod3( light_dir, norm ), 0.0f, 1.0f );
		float illum_specular = clamp( dot_prod3( reflect3( light_dir, norm ), eye_dir ), 0.0f, 1.0f );
		float powered_illum_spec = illum_specular*illum_specular;
		powered_illum_spec *= powered_illum_spec;
		powered_illum_spec *= powered_illum_spec;
		out.color[0] = ambi_color * 0.01f + diff_color * illum_diffuse + spec_color * powered_illum_spec;
		// out.color[0] = ( vec4(norm, 1.0f) + vec4(1.0f, 1.0f, 1.0f, 1.0f) ) * 0.5f;
		out.color[0][3] = 1.0f;

		return true;
	}
	virtual h_pixel_shader create_clone()
	{
		return h_pixel_shader(new morph_ps(*this));
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

class morph: public quick_app{
public:
	morph(): quick_app( create_wtl_application() ), num_frames(0), accumulate_time(0.0f), fps(0.0f) {}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		cout << "Creating window and device ..." << endl;

		string title( "Sample: Morph" );
		impl->main_window()->set_title( title );

		std::string dll_name = "salviax_";
		dll_name += (PRESENTER_NAME);
		dll_name += "_presenter";
#ifdef EFLIB_DEBUG
		dll_name += "_d";
#endif
		dll_name += ".dll";

		shared_ptr<dynamic_lib> presenter_dll = eflib::dynamic_lib::load(dll_name);
		EFLIB_IMPORT_DLL_FUNCTION(
			void (*)(salviar::h_device&, void*),
			create_presenter_device,
			presenter_dll,
			salviax_create_presenter_device
			);
		
		boost::any view_handle_any = impl->main_window()->view_handle();
		create_presenter_device(present_dev, *boost::unsafe_any_cast<void*>( &view_handle_any ) );

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

		cout << "Loading mesh ... " << endl;
		char const* src_file = "../../resources/models/morph/src.dae";
		char const* dst_file = "../../resources/models/morph/dst.dae";
		morph_mesh = create_morph_mesh_from_collada( hsr.get(), src_file, dst_file );

		assert(morph_mesh);

#ifdef SASL_VERTEX_SHADER_ENABLED
		cout << "Compiling vertex shader ... " << endl;
		morph_sc = shader_code::create_and_log( morph_vs_code, lang_vertex_shader );
#endif

		pvs.reset( new morph_vs() );
		pps.reset( new morph_ps() );
		pbs.reset( new bs() );

		f = fopen("indices.txt", "w");
		fclose(f);
	}
	/** @} */

	void on_draw(){
		present_dev->present(*pdsurf);
	}

	void on_idle(){
		static int frame_count = 0;
		// measure statistics
		++frame_count;
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

		vec4 camera_pos = vec4( 0.0f, 70.0f, -160.0f, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera_pos.xyz(), vec3(0.0f, 35.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);

		static float ang = 0.0f;
		ang += elapsed_time/2.0f;
		vec4 lightPos( sin(ang)*160.0f, 40.0f, cos(ang)*160.0f, 1.0f );

		hsr->set_pixel_shader(pps);
		hsr->set_blend_shader(pbs);
		
		static float blendWeight = 1.0f;
		blendWeight += elapsed_time/5.0f;
		blendWeight = fmodf( blendWeight, 2.0f );
		
		float finalBlendWeight = blendWeight > 1.0f ? 2.0f - blendWeight : blendWeight;

		for(float i = 0 ; i < 1 ; i ++)
		{
			world = mat44::identity();
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			hsr->set_rasterizer_state(rs_back);

			// C++ vertex shader and SASL vertex shader are all available.
#ifdef SASL_VERTEX_SHADER_ENABLED
			hsr->set_vertex_shader_code( morph_sc );

			hsr->set_vs_variable( "wvpMatrix", &wvp );
			hsr->set_vs_variable( "eyePos", &camera_pos );
			hsr->set_vs_variable( "lightPos", &lightPos );
			hsr->set_vs_variable( "blendWeight", &finalBlendWeight );
#else
			pvs->set_constant( _T("wvpMatrix"), &wvp );
			pvs->set_constant( _T("eyePos"), &camera_pos );
			pvs->set_constant( _T("lightPos"), &lightPos );

			hsr->set_vertex_shader(pvs);
#endif
			morph_mesh->render();
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

	mesh_ptr morph_mesh;

	shared_ptr<shader_code> morph_sc;

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
	morph loader;
	return loader.run();
}