#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/presenter_dev.h>
#include <salviar/include/shader.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/resource/mesh/sa/material.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_collada.h>

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

#define SASL_VERTEX_SHADER_ENABLED

char const* astro_boy_vs_code =
"float4x4 wvpMatrix; \r\n"
"float4   eyePos; \r\n"
"float4	  lightPos; \r\n"
"int	  boneCount;\r\n"
"float4x4 boneMatrices[boneCount]; \r\n"
"float4x4 invMatrices[boneCount]; \r\n"
" \r\n"
"struct VSIn{ \r\n"
"	float3 pos: POSITION; \r\n"
"	float3 norm: NORMAL; \r\n"
"	int4   indices: BLEND_INDICES; \r\n"
"	float4 weights: BLEND_WEIGHTS; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: sv_position; \r\n"
"	float4 norm: TEXCOORD1; \r\n"
"	float4 lightDir: TEXCOORD2; \r\n"
"	float4 eyeDir: TEXCOORD3; \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
"	float4 nor_v4f32 = float4(in.norm, 0.0f); \r\n"
"	float4 pos_v4f32 = float4(in.pos, 1.0f); \r\n"
"	float4 skin_pos = float4(0.0f, 0.0f, 0.0f, 0.0f); \r\n"
"	float4 skin_nor = float4(0.0f, 0.0f, 0.0f, 0.0f); \r\n"
"	for(int i = 0; i < 4; ++i){\r\n"
"		float4 w = in.weights[i].xxxx; \r\n"
"		int boneId = in.indices[i]; \r\n"
"		if(boneId == -1){ break; } \r\n"
"		float4 posInBoneSpace = mul(invMatrices[boneId], pos_v4f32); \r\n"
"		skin_pos += ( mul(boneMatrices[boneId], posInBoneSpace) * w ); \r\n"
"		float4 norInBoneSpace = mul(invMatrices[boneId], nor_v4f32); \r\n"
"		skin_nor += ( mul(boneMatrices[boneId], norInBoneSpace) * w ); \r\n"
"	}\r\n"
"	out.pos = mul(skin_pos, wvpMatrix); \r\n"
"	out.norm = skin_nor;\r\n"
"	out.lightDir = lightPos-skin_pos; \r\n"
"	out.eyeDir = eyePos-skin_pos; \r\n"
"	return out; \r\n"
"} \r\n"
;

FILE* f = NULL;

class astro_boy_vs : public vertex_shader
{
	mat44 wvp;
	vec4 light_pos, eye_pos;
	vector<mat44> invMatrices;
	vector<mat44> boneMatrices;
public:
	astro_boy_vs():wvp(mat44::identity()){
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

	astro_boy_vs(const mat44& wvp):wvp(wvp){}
	void shader_prog(const vs_input& in, vs_output& out)
	{
		vec4 pos = in.attribute(0);
		vec4 nor = in.attribute(1);

		out.position() = out.attribute(0) = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		pos.w(1.0f);
		nor.w(0.0f);
		/*out.position = pos;
		out.attribute(0) = nor;*/
		
		for(int i = 0; i < 4; ++i)
		{
			union {float f; int i;} f2i;
			f2i.f = in.attribute(3)[i];
			float w = in.attribute(4)[i];
			int boneIndex = f2i.i;
			if(boneIndex == -1){break;}
			// fprintf(f, "%2d ", boneIndex);
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
		fprintf( f, "(%8.4f %8.4f %8.4f %8.4f)",
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

class astro_boy_ps : public pixel_shader
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

	astro_boy_ps()
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
		vec4 diff_color = vec4(1.0f, 1.0f, 1.0f, 1.0f); // diffuse;

		if( tex_ ){
			diff_color = tex2d(*sampler_, 0).get_vec4();
		}

		vec3 norm( normalize3( in.attribute(0).xyz() ) );
		vec3 light_dir( normalize3( in.attribute(1).xyz() ) );
		vec3 eye_dir( normalize3( in.attribute(2).xyz() ) );

		float illum_diffuse = clamp( dot_prod3( light_dir, norm ), 0.0f, 1.0f );
		float illum_specular = clamp( dot_prod3( reflect3( light_dir, norm ), eye_dir ), 0.0f, 1.0f );

		out.color[0] = ambient * 0.01f + diff_color * illum_diffuse + specular * illum_specular;
		out.color[0] = diff_color * illum_diffuse;
		//out.color[0] = ( vec4(norm, 1.0f) + vec4(1.0f, 1.0f, 1.0f, 1.0f) ) * 0.5f;
		out.color[0][3] = 1.0f;

		return true;
	}
	virtual pixel_shader_ptr create_clone()
	{
		return pixel_shader_ptr(new astro_boy_ps(*this));
	}
	virtual void destroy_clone(pixel_shader_ptr& ps_clone)
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

class astro_boy: public quick_app{
public:
	astro_boy(): quick_app( create_wtl_application() ), num_frames(0), accumulate_time(0.0f), fps(0.0f) {}

protected:
	/** Event handlers @{ */
	virtual void on_create(){

		cout << "Creating window and device ..." << endl;

		string title( "Sample: Astro Boy" );
		impl->main_window()->set_title( title );
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

		raster_desc rs_desc;
		rs_desc.cm = cull_none;
		rs_back.reset(new raster_state(rs_desc));

		cout << "Loading mesh ... " << endl;
		astro_boy_mesh = create_mesh_from_collada( hsr.get(), "../../resources/models/astro_boy/astroBoy_walk_Maya.dae" );

#ifdef SASL_VERTEX_SHADER_ENABLED
		cout << "Compiling vertex shader ... " << endl;
		astro_boy_sc = compile( astro_boy_vs_code, lang_vertex_shader );
#endif

		pvs.reset( new astro_boy_vs() );
		pps.reset( new astro_boy_ps() );
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

		vec4 camera_pos = vec4( 0.0f, 10.0f, 14.0f, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera_pos.xyz(), vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), 1.0f, 0.1f, 1000.0f);

		static float ang = 0.0f;
		ang += elapsed_time/3.0f;
		vec4 lightPos( sin(ang)*15.0f, 10.0f, cos(ang)*15.0f, 1.0f );

		hsr->set_pixel_shader(pps);
		hsr->set_blend_shader(pbs);
		
		for(float i = 0 ; i < 1 ; i ++)
		{
			mat_translate(world , -0.5f + i * 0.5f, 0, -0.5f + i * 0.5f);
			mat_mul(wvp, world, mat_mul(wvp, view, proj));

			hsr->set_rasterizer_state(rs_back);

			static float cur_time = 0.0f;
			cur_time = fmodf(cur_time+elapsed_time/1.5f, 1.0f);
			astro_boy_mesh->set_time(cur_time+0.05f);

			vector<mat44> boneMatrices = astro_boy_mesh->joint_matrices();
			vector<mat44> boneInvMatrices = astro_boy_mesh->bind_inv_matrices();
			int boneSize = (int)boneMatrices.size();

			// C++ vertex shader and SASL vertex shader are all available.
#ifdef SASL_VERTEX_SHADER_ENABLED
			hsr->set_vertex_shader_code( astro_boy_sc );

			hsr->set_vs_variable( "wvpMatrix", &wvp );
			hsr->set_vs_variable( "eyePos", &camera_pos );
			hsr->set_vs_variable( "lightPos", &lightPos );

			hsr->set_vs_variable( "boneCount", &boneSize );
			hsr->set_vs_variable_pointer( "boneMatrices", &boneMatrices[0], sizeof(mat44)*boneMatrices.size() );
			hsr->set_vs_variable_pointer( "invMatrices", &boneInvMatrices[0], sizeof(mat44)*boneInvMatrices.size() );
#else
			pvs->set_constant( _T("wvpMatrix"), &wvp );
			pvs->set_constant( _T("eyePos"), &camera_pos );
			pvs->set_constant( _T("lightPos"), &lightPos );

			// hsr->set_constant( "boneCount", &boneSize );
			pvs->set_constant( _T("boneMatrices"), &boneMatrices );
			pvs->set_constant( _T("invMatrices"), &boneInvMatrices );

			hsr->set_vertex_shader(pvs);
#endif
			f = fopen("indices.txt", "a");
			fprintf(f, "FRAME %d", frame_count);
			for( size_t i_mesh = 0; i_mesh < 1 /*astro_boy_mesh->submesh_count()*/; ++i_mesh )
			{
				astro_boy_mesh->render(i_mesh);
			}
			fclose(f);
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

	skin_mesh_ptr astro_boy_mesh;

	shader_object_ptr astro_boy_sc;

	vertex_shader_ptr	pvs;
	pixel_shader_ptr	pps;
	blend_shader_ptr	pbs;

	raster_state_ptr rs_back;

	surface_ptr display_surf;
	surface* pdsurf;

	uint32_t num_frames;
	float accumulate_time;
	float fps;

	timer timer;
	/** @} */
};

int main( int /*argc*/, TCHAR* /*argv*/[] ){
	astro_boy loader;
	return loader.run();
}