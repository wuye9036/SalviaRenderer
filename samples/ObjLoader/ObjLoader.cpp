#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

#include <salviar/include/presenter_dev.h>
#include <salviar/include/shader.h>
#include <salviar/include/shader_code.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/colors.h>

#include <salviax/include/resource/mesh/sa/mesh_io.h>

#include <salviau/include/common/timer.h>
#include <salviau/include/common/window.h>

#define PRESENTER_NAME "d3d9"

using namespace eflib;
using namespace salviar;
using namespace salviax;
using namespace salviax::resource;
using namespace salviau;

using boost::shared_ptr;
using boost::static_pointer_cast;

using std::cout;
using std::endl;

char const* vs_code = 
"float4x4	wvpMatrix; \r\n"
"float4		lightPos0; \r\n"
"float4		lightPos1; \r\n"
"float4		lightPos2; \r\n"
"struct VSIn{ \r\n"
"	float4 pos: POSITION; \r\n"
"	float4 norm: NORMAL; \r\n"
"}; \r\n"
"struct VSOut{ \r\n"
"	float4 pos: sv_position; \r\n"
"	float4 norm: TEXCOORD(0); \r\n"
"	float4 lightDir0: TEXCOORD(1); \r\n"
"	float4 lightDir1: TEXCOORD(2); \r\n"
"	float4 lightDir2: TEXCOORD(3); \r\n"
"}; \r\n"
"VSOut vs_main(VSIn in){ \r\n"
"	VSOut out; \r\n"
"	out.norm = in.norm; \r\n"
"	out.pos = mul(in.pos, wvpMatrix); \r\n"
"	out.lightDir0 = lightPos0 - in.pos;"
"	out.lightDir1 = lightPos1 - in.pos;"
"	out.lightDir2 = lightPos2 - in.pos;"
"	return out; \r\n"
"} \r\n"
;

class ps : public pixel_shader
{
public:
	ps()
	{}
	bool shader_prog(const vs_output& in, ps_output& out)
	{
		vec3 lightDir0 = in.attributes[1].xyz();
		vec3 lightDir1 = in.attributes[2].xyz();
		vec3 lightDir2 = in.attributes[3].xyz();

		vec3 norm = in.attributes[0].xyz();

		float invLight0Distance = 1.0f / lightDir0.length();
		float invLight1Distance = 1.0f / lightDir1.length();
		float invLight2Distance = 1.0f / lightDir2.length();

		vec3 normalized_norm = normalize3( norm );
		vec3 normalized_lightDir0 = lightDir0 * invLight0Distance;
		vec3 normalized_lightDir1 = lightDir1 * invLight1Distance;
		vec3 normalized_lightDir2 = lightDir2 * invLight2Distance;

		float refl0 = dot_prod3( normalized_norm, normalized_lightDir0 );
		float refl1 = dot_prod3( normalized_norm, normalized_lightDir1 );
		float refl2 = dot_prod3( normalized_norm, normalized_lightDir2 );

		out.color[0] = clampss(
			vec4(0.7f, 0.1f, 0.3f, 1.0f ) * refl0 * invLight0Distance * invLight0Distance +
			vec4(0.1f, 0.3f, 0.7f, 1.0f ) * refl1 * invLight1Distance * invLight1Distance +
			vec4(0.3f, 0.7f, 0.1f, 1.0f ) * refl2 * invLight2Distance * invLight2Distance
			, 0.0f, 1.0f
			)
			;

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

class obj_loader: public quick_app{
public:
	obj_loader(): quick_app( create_wtl_application() ){}

protected:
	/** Event handlers @{ */
	virtual void on_create(){
		cout << "Window Created!" << endl;
	}
	/** @} */

protected:
	/** Properties @{ */
	h_device present_dev;
	h_renderer hsr;

	h_texture sm_tex;
	h_mesh planar_mesh;

	h_pixel_shader pps;
	h_blend_shader pbs;

	h_rasterizer_state rs_back;

	h_surface display_surf;
	surface* pdsurf;

	uint32_t num_frames;
	float accumulate_time;
	float fps;

	timer_t timer;
	/** @} */
};

int main( int argc, TCHAR* argv[] ){
	obj_loader loader;
	return loader.run();
}