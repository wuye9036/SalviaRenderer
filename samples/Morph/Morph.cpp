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
#include <salviax/include/resource/mesh/sa/material.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_collada.h>

#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#include <eflib/include/platform/dl_loader.h>

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
"	out.pos = mul( morphed_pos_v4f32, wvpMatrix ); \r\n"
"	out.norm = float4( in.norm0+(in.norm1-in.norm0)*blendWeight.xxx, 0.0f );\r\n"
"	out.lightDir = lightPos-morphed_pos_v4f32; \r\n"
"	out.eyeDir = eyePos-morphed_pos_v4f32; \r\n"
"	return out; \r\n"
"} \r\n"
;

FILE* f = NULL;

class morph_vs : public cpp_vertex_shader
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
		vec4 pos = in.attribute(0);
		vec4 nor = in.attribute(1);

		out.position() = out.attribute(0) = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		pos.w(1.0f);
		nor.w(0.0f);

		for(int i = 0; i < 4; ++i)
		{
			union {float f; int i;} f2i;
			f2i.f = in.attribute(3)[i];
			float w = in.attribute(4)[i];
			int boneIndex = f2i.i;
			if(boneIndex == -1){break;}
			vec4 skin_pos;
			vec4 skin_nor;
			transform(skin_pos, invMatrices[boneIndex], pos);
			transform(skin_pos, boneMatrices[boneIndex], skin_pos);
			transform(skin_nor, invMatrices[boneIndex], nor);
			transform(skin_nor, boneMatrices[boneIndex], skin_nor);
			out.position() += (skin_pos*w);
			out.attribute(0) += (skin_nor*w);
		}
		
#ifdef EFLIB_DEBUG
		fprintf(f, "(%8.4f %8.4f %8.4f %8.4f)",
			out.position().x(), out.position().y(), out.position().z(), out.position().w()
			);
#endif
		transform(out.position(), out.position(), wvp);
#ifdef EFLIB_DEBUG
		fprintf(f, "\n");
#endif

		// out.attribute(0) = in.attribute(1);
		out.attribute(1) = in.attribute(2);
		out.attribute(2) = light_pos - pos;
		out.attribute(3) = eye_pos - pos;
	}

	uint32_t num_output_attributes() const{
		return 4;
	}

	uint32_t output_attribute_modifiers(uint32_t) const{
		return salviar::vs_output::am_linear;
	}
};

class morph_ps : public cpp_pixel_shader
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

		vec3 norm( normalize3( in.attribute(0).xyz() ) );
		vec3 light_dir( normalize3( in.attribute(1).xyz() ) );
		vec3 eye_dir( normalize3( in.attribute(2).xyz() ) );

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
	virtual cpp_pixel_shader_ptr create_clone()
	{
		return cpp_pixel_shader_ptr(new morph_ps(*this));
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

class morph: public quick_app{
public:
	morph(): quick_app( create_wtl_application() ), num_frames(0), accumulate_time(0.0f), fps(0.0f) {}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		cout << "Creating window and device ..." << endl;

		string title( "Sample: Morph" );
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

		cout << "Loading mesh ... " << endl;
		char const* src_file = "../../resources/models/morph/src.dae";
		char const* dst_file = "../../resources/models/morph/dst.dae";
		morph_mesh = create_morph_mesh_from_collada( renderer_.get(), src_file, dst_file );

		assert(morph_mesh);

#ifdef SASL_VERTEX_SHADER_ENABLED
		cout << "Compiling vertex shader ... " << endl;
		morph_sc = compile( morph_vs_code, lang_vertex_shader );
#endif

		pvs.reset( new morph_vs() );
		pps.reset( new morph_ps() );
		pbs.reset( new bs() );

		f = fopen("indices.txt", "w");
		fclose(f);
	}
	/** @} */

	void on_draw()
    {
		swap_chain_->present();
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

        renderer_->clear_color(color_surface_, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
		renderer_->clear_depth_stencil(ds_surface_, 1.0f, 0);

		vec4 camera_pos = vec4( 0.0f, 70.0f, -160.0f, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera_pos.xyz(), vec3(0.0f, 35.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);

		static float ang = 0.0f;
		ang += elapsed_time/2.0f;
		vec4 lightPos( sin(ang)*160.0f, 40.0f, cos(ang)*160.0f, 1.0f );

		renderer_->set_pixel_shader(pps);
		renderer_->set_blend_shader(pbs);
		
		static float blendWeight = 1.0f;
		blendWeight += elapsed_time/5.0f;
		blendWeight = fmodf( blendWeight, 2.0f );
		
		float finalBlendWeight = blendWeight > 1.0f ? 2.0f - blendWeight : blendWeight;

		for(float i = 0 ; i < 1 ; i ++)
		{
			world = mat44::identity();
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			renderer_->set_rasterizer_state(rs_back);

			// C++ vertex shader and SASL vertex shader are all available.
#ifdef SASL_VERTEX_SHADER_ENABLED
			renderer_->set_vertex_shader_code( morph_sc );

			renderer_->set_vs_variable( "wvpMatrix", &wvp );
			renderer_->set_vs_variable( "eyePos", &camera_pos );
			renderer_->set_vs_variable( "lightPos", &lightPos );
			renderer_->set_vs_variable( "blendWeight", &finalBlendWeight );
#else
			pvs->set_constant( _T("wvpMatrix"), &wvp );
			pvs->set_constant( _T("eyePos"), &camera_pos );
			pvs->set_constant( _T("lightPos"), &lightPos );

			renderer_->set_vertex_shader(pvs);
#endif
			morph_mesh->render();
		}

		impl->main_window()->refresh();
	}

protected:
	/** Properties @{ */
	swap_chain_ptr          swap_chain_;
	renderer_ptr            renderer_;
    surface_ptr             ds_surface_;
    surface_ptr             color_surface_;

	mesh_ptr                morph_mesh;

	shader_object_ptr       morph_sc;

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
	morph loader;
	return loader.run();
}