#include <salvia/core/renderer.h>

#include <salvia/core/async_renderer.h>
#include <salvia/core/binary_modules.h>
#include <salvia/core/sync_renderer.h>
#include <salvia/resource/sampler_api.h>
#include <salvia/shader/shader_impl.h>
#include <salvia/shader/shader_object.h>

#include <eflib/platform/dl_loader.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace salvia::shader;

using eflib::dynamic_lib;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;
using std::vector;

namespace salvia::core {

#define USE_ASYNC_RENDERER
renderer_ptr create_software_renderer() {
#if defined(USE_ASYNC_RENDERER)
  return create_async_renderer();
#else
  return create_sync_renderer();
#endif
}

renderer_ptr create_benchmark_renderer() { return create_sync_renderer(); }

shader_object_ptr compile(std::string const &code, shader_profile const &profile,
                          shader_log_ptr &logs) {
  vector<external_function_desc> external_funcs;
  external_funcs.push_back(external_function_desc((void *)&tex2Dlod, "sasl.vs.tex2d.lod", true));
  external_funcs.push_back(
      external_function_desc((void *)&texCUBElod, "sasl.vs.texCUBE.lod", true));
  external_funcs.push_back(external_function_desc((void *)&tex2Dlod_ps, "sasl.ps.tex2d.lod", true));
  external_funcs.push_back(
      external_function_desc((void *)&tex2Dgrad_ps, "sasl.ps.tex2d.grad", true));
  external_funcs.push_back(
      external_function_desc((void *)&tex2Dbias_ps, "sasl.ps.tex2d.bias", true));
  external_funcs.push_back(
      external_function_desc((void *)&tex2Dproj_ps, "sasl.ps.tex2d.proj", true));

  shader_object_ptr ret;
  modules::host::compile(ret, logs, code, profile, external_funcs);

  return ret;
}

shader_object_ptr compile(std::string const &code, shader_profile const &profile) {
  shader_log_ptr log;
  shader_object_ptr ret = compile(code, profile, log);

  if (!ret) {
    cout << "Shader was compiled failed!" << endl;
    for (size_t i = 0; i < log->count(); ++i) {
      cout << log->log_string(i) << endl;
    }
  }

  return ret;
}

shader_object_ptr compile(std::string const &code, languages lang) {
  shader_profile prof;
  prof.language = lang;
  return compile(code, prof);
}

shader_object_ptr compile_from_file(std::string const &file_name, shader_profile const &profile,
                                    shader_log_ptr &logs) {
  vector<external_function_desc> external_funcs;
  external_funcs.push_back(external_function_desc((void *)&tex2Dlod, "sasl.vs.tex2d.lod", true));
  external_funcs.push_back(
      external_function_desc((void *)&texCUBElod, "sasl.vs.texCUBE.lod", true));
  external_funcs.push_back(external_function_desc((void *)&tex2Dlod_ps, "sasl.ps.tex2d.lod", true));
  external_funcs.push_back(
      external_function_desc((void *)&tex2Dgrad_ps, "sasl.ps.tex2d.grad", true));
  external_funcs.push_back(
      external_function_desc((void *)&tex2Dbias_ps, "sasl.ps.tex2d.bias", true));
  external_funcs.push_back(
      external_function_desc((void *)&tex2Dproj_ps, "sasl.ps.tex2d.proj", true));

  shader_object_ptr ret;
  modules::host::compile_from_file(ret, logs, file_name, profile, external_funcs);

  return ret;
}

shader_object_ptr compile_from_file(std::string const &file_name, shader_profile const &profile) {
  shader_log_ptr log;
  shader_object_ptr ret = compile_from_file(file_name, profile, log);

  if (!ret) {
    cout << "Shader was compiled failed!" << endl;
    for (size_t i = 0; i < log->count(); ++i) {
      cout << log->log_string(i) << endl;
    }
  }

  return ret;
}

shader_object_ptr compile_from_file(std::string const &file_name, languages lang) {
  shader_profile prof;
  prof.language = lang;
  return compile_from_file(file_name, prof);
}

} // namespace salvia::core
