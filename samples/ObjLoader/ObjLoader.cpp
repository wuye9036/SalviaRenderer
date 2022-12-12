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
#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>

#include <salviau/include/common/sample_app.h>
#include <salviau/include/common/path.h>

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

char const* cup_vs_code =
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

class cup_ps : public cpp_pixel_shader
{
	salviar::sampler_ptr sampler_;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	int shininess;

public:
	cup_ps()
	{
		declare_constant(_T("Ambient"),   ambient );
		declare_constant(_T("Diffuse"),   diffuse );
		declare_constant(_T("Specular"),  specular );
		declare_constant(_T("Shininess"), shininess );
        declare_sampler(_T("Sampler"), sampler_);
	}

	vec4 to_color(vec3 const& v)
	{
		return vec4(
			(v.x() + 1.0f) / 2.0f,
			(v.y() + 1.0f) / 2.0f,
			(v.z() + 1.0f) / 2.0f,
			1.0f
			);
	}

	bool shader_prog(const vs_output& in, ps_output& out)
	{
		color_rgba32f tex_color(1.0f, 1.0f, 1.0f, 1.0f);
		if( sampler_ )
		{
			tex_color = tex2d(*sampler_ , 0);
		}
		vec3 norm( normalize3( in.attribute(1).xyz() ) );
		vec3 light_dir( normalize3( in.attribute(2).xyz() ) );
		vec3 eye_dir( normalize3( in.attribute(3).xyz() ) );

		float illum_diffuse = clamp( dot_prod3( light_dir, norm ), 0.0f, 1.0f );
		float illum_specular = clamp( dot_prod3( reflect3( light_dir, norm ), eye_dir ), 0.0f, 1.0f );
		vec4 illum = ambient + diffuse * illum_diffuse + specular * illum_specular;

		out.color[0] = tex_color.get_vec4() * illum;
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

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 3 : 2500;
int const TEST_FRAME_COUNT		= 6;

class obj_loader: public sample_app
{
public:
	obj_loader(): sample_app( "ObjLoader" ){}

protected:
	void on_init() override
	{
        create_devices_and_targets(data_->screen_width, data_->screen_height, 1, pixel_format_color_bgra8, pixel_format_color_rg32f);
		data_->renderer->set_viewport(data_->screen_vp);
		
		raster_desc rs_desc;
		rs_desc.cm = cull_back;
		rs_back.reset(new raster_state(rs_desc));

		cup_vs = compile(cup_vs_code, lang_vertex_shader);
		cup_mesh = create_mesh_from_obj( data_->renderer.get(), find_path("cup/cup.obj"), true );

		pps.reset( new cup_ps() );
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

		if(!cup_vs){ return; }

		float scene_sec = 0.0f;

		switch(data_->mode)
		{
		case app_modes::benchmark:
			scene_sec = static_cast<float>(data_->frame_count) * 0.015f;
			break;
		case app_modes::test:
			scene_sec = static_cast<float>(data_->frame_count) * 1.5f;
			break;
		default:
			scene_sec = static_cast<float>(data_->total_elapsed_sec);
			break;
		}

		float angle = scene_sec * 60.0f * (static_cast<float>(TWO_PI) / 360.0f) * 0.15f;

		vec3 camera(cos(angle) * 2.0f, 0.5f, sin(angle) * 2.0f);
		mat44 world(mat44::identity()), view, proj, wvp;

		mat_lookat(view, camera, vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat_perspective_fov(proj, static_cast<float>(HALF_PI), data_->screen_aspect_ratio, 0.1f, 100.0f);

		vec4 lightPos(sin(-angle * 1.5f) * 2.2f, 0.15f, cos(angle * 0.9f) * 1.8f, 0.0f);

		data_->renderer->set_pixel_shader(pps);
		data_->renderer->set_blend_shader(pbs);

		mat_translate(world , -0.5f, 0, -0.5f);
		mat_mul(wvp, world, mat_mul(wvp, view, proj));

		profiling("Rendering", [&](){
			data_->renderer->set_rasterizer_state(rs_back);
			data_->renderer->set_vertex_shader_code( cup_vs );
			data_->renderer->set_vs_variable( "wvpMatrix", &wvp );
			vec4 camera_pos = vec4( camera, 1.0f );
			data_->renderer->set_vs_variable( "eyePos", &camera_pos );
			data_->renderer->set_vs_variable( "lightPos", &lightPos );

			for( size_t i_mesh = 0; i_mesh < cup_mesh.size(); ++i_mesh )
			{
				mesh_ptr cur_mesh = cup_mesh[i_mesh];

				shared_ptr<obj_material> mtl
					= dynamic_pointer_cast<obj_material>( cur_mesh->get_attached() );
				pps->set_constant( _T("Ambient"),  &mtl->ambient );
				pps->set_constant( _T("Diffuse"),  &mtl->diffuse );
				pps->set_constant( _T("Specular"), &mtl->specular );
				pps->set_constant( _T("Shininess"),&mtl->ambient );

				sampler_desc desc;
				desc.min_filter = filter_linear;
				desc.mag_filter = filter_linear;
				desc.mip_filter = filter_linear;
				desc.addr_mode_u = address_clamp;
				desc.addr_mode_v = address_clamp;
				desc.addr_mode_w = address_clamp;
                
				if(mtl->tex)
				{
					pps->set_sampler(_T("Sampler"), data_->renderer->create_sampler(desc, mtl->tex));
				}
				else
				{
					pps->set_sampler(_T("Sampler"), sampler_ptr());
				}

				cur_mesh->render();
			}
		});
	}

protected:
	vector<mesh_ptr>        cup_mesh;

	shader_object_ptr       plane_vs;
	shader_object_ptr       cup_vs;

	cpp_pixel_shader_ptr    pps;
	cpp_blend_shader_ptr    pbs;

	raster_state_ptr        rs_back;
};

EFLIB_MAIN(argc, argv)
{
	obj_loader loader;
	loader.init(argc, const_cast<std::_tchar const**>(argv));
	loader.run();
	return 0;
}