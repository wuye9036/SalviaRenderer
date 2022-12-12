#include <salviau/include/common/sample_app.h>
#include <salviau/include/common/path.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>
#include <salviar/include/texture.h>

#include <salviax/include/swap_chain/swap_chain.h>
#include <salviax/include/resource/mesh/sa/material.h>
#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviax/include/resource/mesh/sa/mesh_io_collada.h>
#include <salviax/include/resource/texture/tex_io.h>

#include <eflib/platform/main.h>

#include <vector>

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

class astro_boy_vs : public cpp_vertex_shader
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
		
		transform(out.position(), out.position(), wvp);

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
    
    virtual cpp_shader_ptr clone()
	{
        typedef std::remove_pointer<decltype(this)>::type this_type;
		return cpp_shader_ptr(new this_type(*this));
	}

};

class astro_boy_ps : public cpp_pixel_shader
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	int shininess;

public:
	astro_boy_ps()
	{
		declare_constant(_T("Ambient"),   ambient );
		declare_constant(_T("Diffuse"),   diffuse );
		declare_constant(_T("Specular"),  specular );
		declare_constant(_T("Shininess"), shininess );
	}

	bool shader_prog(const vs_output& in, ps_output& out)
	{
		vec4 diff_color = vec4(1.0f, 1.0f, 1.0f, 1.0f); // diffuse;

		vec3 norm( normalize3( in.attribute(0).xyz() ) );
		vec3 light_dir( normalize3( in.attribute(1).xyz() ) );
		vec3 eye_dir( normalize3( in.attribute(2).xyz() ) );

		float illum_diffuse = clamp( dot_prod3( light_dir, norm ), 0.0f, 1.0f );
		float illum_specular = clamp( dot_prod3( reflect3( light_dir, norm ), eye_dir ), 0.0f, 1.0f );

		out.color[0] = ambient * 0.01f + diff_color * illum_diffuse + specular * illum_specular;
		out.color[0] = diff_color * illum_diffuse;
		// out.color[0] = ( vec4(norm, 1.0f) + vec4(1.0f, 1.0f, 1.0f, 1.0f) ) * 0.5f;

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

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 10 : 1500;
int const TEST_FRAME_COUNT		= 15;

class astro_boy: public sample_app
{
public:
	astro_boy(): sample_app("AstroBoy")
	{}

protected:
	virtual void on_init()
	{
		create_devices_and_targets(data_->screen_width, data_->screen_height, 1, pixel_format_color_bgra8, pixel_format_color_rg32f);
        
		data_->renderer->set_viewport(data_->screen_vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

		cout << "Loading mesh ... " << endl;
		astro_boy_mesh = create_mesh_from_collada( data_->renderer.get(), find_path("astro_boy/astroBoy_walk_Maya.dae") );

#ifdef SASL_VERTEX_SHADER_ENABLED
		cout << "Compiling vertex shader ... " << endl;
		astro_boy_sc = compile( astro_boy_vs_code, lang_vertex_shader );
#endif

		pvs.reset( new astro_boy_vs() );
		pps.reset( new astro_boy_ps() );
		pbs.reset( new bs() );

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

		vec4 camera_pos = vec4( 0.0f, 10.0f, 14.0f, 1.0f );

		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera_pos.xyz(), vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), data_->screen_aspect_ratio, 0.1f, 1000.0f);

		float ang = 0.0f;
		float ani_time = 0.0f;
		switch(data_->mode)
		{
		case app_modes::test:
			ang = static_cast<float>(data_->frame_count) / 7.0f;
			ani_time = static_cast<float>(data_->frame_count) / 10.0F;
			break;
		case app_modes::benchmark:
			ang = static_cast<float>(data_->frame_count) / 7.0f;
			ani_time = static_cast<float>(data_->frame_count) / 10.0f;
			break;
		default:
			ang = static_cast<float>(data_->total_elapsed_sec / 3.0f);
			ani_time = static_cast<float>(data_->total_elapsed_sec / 1.0f);
			break;
		}
		ani_time = fmodf( ani_time, astro_boy_mesh->animation_length() );
		
		vec4 lightPos( sin(ang)*15.0f, 10.0f, cos(ang)*15.0f, 1.0f );

		data_->renderer->set_pixel_shader(pps);
		data_->renderer->set_blend_shader(pbs);
		
		mat_translate(world , -0.5f, 0, -0.5f);
		mat_mul(wvp, world, mat_mul(wvp, view, proj));

		data_->renderer->set_rasterizer_state(rs_back);

		astro_boy_mesh->set_time(ani_time);

		vector<mat44> boneMatrices = astro_boy_mesh->joint_matrices();
		vector<mat44> boneInvMatrices = astro_boy_mesh->bind_inv_matrices();
		int boneSize = (int)boneMatrices.size();

		profiling("Rendering", [&](){
			// C++ vertex shader and SASL vertex shader are all available.
#ifdef SASL_VERTEX_SHADER_ENABLED
			data_->renderer->set_vertex_shader_code( astro_boy_sc );

			data_->renderer->set_vs_variable( "wvpMatrix", &wvp );
			data_->renderer->set_vs_variable( "eyePos", &camera_pos );
			data_->renderer->set_vs_variable( "lightPos", &lightPos );

			data_->renderer->set_vs_variable( "boneCount", &boneSize );
			data_->renderer->set_vs_variable_pointer( "boneMatrices", &boneMatrices[0], sizeof(mat44)*boneMatrices.size() );
			data_->renderer->set_vs_variable_pointer( "invMatrices", &boneInvMatrices[0], sizeof(mat44)*boneInvMatrices.size() );
#else
			pvs->set_constant( _T("wvpMatrix"), &wvp );
			pvs->set_constant( _T("eyePos"), &camera_pos );
			pvs->set_constant( _T("lightPos"), &lightPos );

			// data_->renderer->set_constant( "boneCount", &boneSize );
			pvs->set_constant( _T("boneMatrices"), &boneMatrices );
			pvs->set_constant( _T("invMatrices"), &boneInvMatrices );

			data_->renderer->set_vertex_shader(pvs);
#endif

			for(uint32_t i_mesh = 0; i_mesh < astro_boy_mesh->submesh_count(); ++i_mesh)
			{
				astro_boy_mesh->render(i_mesh);
			}
		});
	}

protected:
	skin_mesh_ptr           astro_boy_mesh;
	shader_object_ptr       astro_boy_sc;
	cpp_vertex_shader_ptr	pvs;
	cpp_pixel_shader_ptr	pps;
	cpp_blend_shader_ptr	pbs;
	raster_state_ptr        rs_back;
};

EFLIB_MAIN(argc, argv)
{
	astro_boy loader;
	loader.init(argc, const_cast<std::_tchar const**>(argv));
	loader.run();

	return 0;
}