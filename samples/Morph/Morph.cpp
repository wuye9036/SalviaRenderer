#include <salvia/common/colors.h>
#include <salvia/core/sync_renderer.h>
#include <salvia/resource/resource_manager.h>
#include <salvia/resource/texture.h>
#include <salvia/shader/shader.h>
#include <salvia/shader/shader_object.h>
#include <salvia/shader/shader_regs.h>
#include <salviar/include/rasterizer.h>

#include "salvia/ext/resource/mesh/material.h"
#include "salvia/ext/resource/mesh/mesh_io.h"
#include "salvia/ext/resource/mesh/mesh_io_collada.h"
#include <salvia/ext/swap_chain/swap_chain.h>

#include <salvia/utility/common/path.h>
#include <salvia/utility/common/sample_app.h>

#include <eflib/platform/main.h>

#include <vector>

using namespace eflib;
using namespace salvia::core;
using namespace salvia::ext;
using namespace salvia::ext::resource;
using namespace salviau;

using std::dynamic_pointer_cast;
using std::shared_ptr;

using std::cout;
using std::endl;
using std::string;
using std::vector;

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
    "} \r\n";

class morph_vs : public cpp_vertex_shader {
  mat44 wvp;
  vec4 light_pos, eye_pos;
  vector<mat44> invMatrices;
  vector<mat44> boneMatrices;

public:
  morph_vs() : wvp(mat44::identity()) {
    declare_constant(_T("wvpMatrix"), wvp);
    declare_constant(_T("lightPos"), light_pos);
    declare_constant(_T("eyePos"), eye_pos);
    declare_constant(_T("invMatrices"), invMatrices);
    declare_constant(_T("boneMatrices"), boneMatrices);

    bind_semantic("POSITION", 0, 0);
    bind_semantic("NORMAL", 0, 1);
    bind_semantic("TEXCOORD", 0, 2);
    bind_semantic("BLEND_INDICES", 0, 3);
    bind_semantic("BLEND_WEIGHTS", 0, 4);
  }

  morph_vs(const mat44& wvp) : wvp(wvp) {}
  void shader_prog(const vs_input& in, vs_output& out) {
    vec4 pos = in.attribute(0);
    vec4 nor = in.attribute(1);

    out.position() = out.attribute(0) = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    pos.w(1.0f);
    nor.w(0.0f);

    for (int i = 0; i < 4; ++i) {
      union {
        float f;
        int i;
      } f2i;
      f2i.f = in.attribute(3)[i];
      float w = in.attribute(4)[i];
      int boneIndex = f2i.i;
      if (boneIndex == -1) {
        break;
      }
      vec4 skin_pos;
      vec4 skin_nor;
      transform(skin_pos, invMatrices[boneIndex], pos);
      transform(skin_pos, boneMatrices[boneIndex], skin_pos);
      transform(skin_nor, invMatrices[boneIndex], nor);
      transform(skin_nor, boneMatrices[boneIndex], skin_nor);
      out.position() += (skin_pos * w);
      out.attribute(0) += (skin_nor * w);
    }

    transform(out.position(), out.position(), wvp);

    // out.attribute(0) = in.attribute(1);
    out.attribute(1) = in.attribute(2);
    out.attribute(2) = light_pos - pos;
    out.attribute(3) = eye_pos - pos;
  }

  uint32_t num_output_attributes() const { return 4; }

  uint32_t output_attribute_modifiers(uint32_t) const { return salviar::vs_output::am_linear; }

  virtual cpp_shader_ptr clone() {
    typedef std::remove_pointer<decltype(this)>::type this_type;
    return cpp_shader_ptr(new this_type(*this));
  }
};

class morph_ps : public cpp_pixel_shader {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  int shininess;

public:
  morph_ps() {
    declare_constant(_T("Ambient"), ambient);
    declare_constant(_T("Diffuse"), diffuse);
    declare_constant(_T("Specular"), specular);
    declare_constant(_T("Shininess"), shininess);
  }

  bool shader_prog(const vs_output& in, ps_output& out) {
    vec4 ambi_color(0.22f, 0.20f, 0.09f, 1.0f);
    vec4 diff_color(0.75f, 0.75f, 0.25f, 1.0f);
    vec4 spec_color(2.0f, 1.7f, 0.0f, 1.0f);

    vec3 norm(normalize3(in.attribute(0).xyz()));
    vec3 light_dir(normalize3(in.attribute(1).xyz()));
    vec3 eye_dir(normalize3(in.attribute(2).xyz()));

    float illum_diffuse = clamp(dot_prod3(light_dir, norm), 0.0f, 1.0f);
    float illum_specular = clamp(dot_prod3(reflect3(light_dir, norm), eye_dir), 0.0f, 1.0f);
    float powered_illum_spec = illum_specular * illum_specular;
    powered_illum_spec *= powered_illum_spec;
    powered_illum_spec *= powered_illum_spec;
    out.color[0] =
        ambi_color * 0.01f + diff_color * illum_diffuse + spec_color * powered_illum_spec;
    // out.color[0] = ( vec4(norm, 1.0f) + vec4(1.0f, 1.0f, 1.0f, 1.0f) ) * 0.5f;
    out.color[0][3] = 1.0f;

    return true;
  }

  virtual cpp_shader_ptr clone() {
    typedef std::remove_pointer<decltype(this)>::type this_type;
    return cpp_shader_ptr(new this_type(*this));
  }
};

class bs : public cpp_blend_shader {
public:
  bool shader_prog(size_t sample, pixel_accessor& inout, const ps_output& in) {
    color_rgba32f color(in.color[0]);
    inout.color(0, sample, color_rgba32f(in.color[0]));
    return true;
  }

  virtual cpp_shader_ptr clone() {
    typedef std::remove_pointer<decltype(this)>::type this_type;
    return cpp_shader_ptr(new this_type(*this));
  }
};

int const BENCHMARK_FRAME_COUNT = eflib::is_debug_mode ? 3 : 1500;
int const TEST_FRAME_COUNT = 8;

class morph : public sample_app {
public:
  morph() : sample_app("Morph") {}

protected:
  void on_init() override {
    create_devices_and_targets(data_->screen_width,
                               data_->screen_height,
                               1,
                               pixel_format_color_bgra8,
                               pixel_format_color_rg32f);
    data_->renderer->set_viewport(data_->screen_vp);

    raster_desc rs_desc;
    rs_desc.cm = cull_none;
    rs_back.reset(new raster_state(rs_desc));

    cout << "Loading mesh ... " << endl;
    morph_mesh = create_morph_mesh_from_collada(
        data_->renderer.get(), find_path("morph/src.dae"), find_path("morph/dst.dae"));

    assert(morph_mesh);

#ifdef SASL_VERTEX_SHADER_ENABLED
    cout << "Compiling vertex shader ... " << endl;
    morph_sc = compile(morph_vs_code, lang_vertex_shader);
#endif

    pvs.reset(new morph_vs());
    pps.reset(new morph_ps());
    pbs.reset(new bs());

    switch (data_->mode) {
    case app_modes::benchmark: quit_at_frame(BENCHMARK_FRAME_COUNT); break;
    case app_modes::test: quit_at_frame(TEST_FRAME_COUNT); break;
    }
  }

  void on_frame() override {
    profiling("BackBufferClearing", [this]() {
      data_->renderer->clear_color(data_->color_target, color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
      data_->renderer->clear_depth_stencil(data_->ds_target, clear_depth | clear_stencil, 1.0f, 0);
    });

    vec4 camera_pos = vec4(0.0f, 70.0f, -160.0f, 1.0f);

    mat44 world(mat44::identity()), view, proj, wvp;

    mat_lookat(view, camera_pos.xyz(), vec3(0.0f, 35.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f));
    mat_perspective_fov(
        proj, static_cast<float>(HALF_PI), data_->screen_aspect_ratio, 0.1f, 1000.0f);

    float scene_sec = 0.0f;
    switch (data_->mode) {
    case app_modes::benchmark:
      scene_sec = static_cast<float>(data_->frame_count) / (BENCHMARK_FRAME_COUNT - 1);
      break;
    case app_modes::test:
      scene_sec = static_cast<float>(data_->frame_count) / (TEST_FRAME_COUNT - 1);
      break;
    default: scene_sec = static_cast<float>(data_->total_elapsed_sec); break;
    }

    float ang = scene_sec / 2.0f;
    vec4 lightPos(sin(ang) * 160.0f, 40.0f, cos(ang) * 160.0f, 1.0f);

    data_->renderer->set_pixel_shader(pps);
    data_->renderer->set_blend_shader(pbs);

    float blendWeight = 1.0f + scene_sec / 5.0f;
    blendWeight = fmodf(blendWeight, 2.0f);
    float finalBlendWeight = blendWeight > 1.0f ? 2.0f - blendWeight : blendWeight;

    world = mat44::identity();
    mat_mul(wvp, world, mat_mul(wvp, view, proj));

    profiling("Rendering", [&]() {
      data_->renderer->set_rasterizer_state(rs_back);

      // C++ vertex shader and SASL vertex shader are all available.
#ifdef SASL_VERTEX_SHADER_ENABLED
      data_->renderer->set_vertex_shader_code(morph_sc);

      data_->renderer->set_vs_variable("wvpMatrix", &wvp);
      data_->renderer->set_vs_variable("eyePos", &camera_pos);
      data_->renderer->set_vs_variable("lightPos", &lightPos);
      data_->renderer->set_vs_variable("blendWeight", &finalBlendWeight);
#else
			pvs->set_constant( _T("wvpMatrix"), &wvp );
			pvs->set_constant( _T("eyePos"), &camera_pos );
			pvs->set_constant( _T("lightPos"), &lightPos );

			data_->renderer->set_vertex_shader(pvs);
#endif
      morph_mesh->render();
    });
  }

protected:
  mesh_ptr morph_mesh;

  shader_object_ptr morph_sc;

  cpp_vertex_shader_ptr pvs;
  cpp_pixel_shader_ptr pps;
  cpp_blend_shader_ptr pbs;

  raster_state_ptr rs_back;
};

EFLIB_MAIN(argc, argv) {
  morph loader;
  loader.init(argc, const_cast<std::_tchar const**>(argv));
  loader.run();

  return 0;
}