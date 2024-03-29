#include <salvia/core/binary_modules.h>

#include <eflib/platform/dl_loader.h>

#include <cassert>

using namespace salvia::shader;

using eflib::dynamic_lib;

using std::shared_ptr;
using std::string;
using std::vector;

namespace salvia::core {

namespace modules {
static void (*compile_func)(shader_object_ptr&,
                            shader_log_ptr&,
                            string const&,
                            shader_profile const&,
                            vector<external_function_desc> const&) = nullptr;
static void (*compile_from_file_func)(shader_object_ptr&,
                                      shader_log_ptr&,
                                      string const&,
                                      shader_profile const&,
                                      vector<external_function_desc> const&) = nullptr;
static void (*create_host_func)(host_ptr& out) = nullptr;

static shared_ptr<dynamic_lib> host_lib;

static void load_function() {
  assert(!host_lib);
  std::string dll_name;
#if defined(EFLIB_MSVC)
  dll_name = "sasl_host";
#elif defined(EFLIB_MINGW) || defined(EFLIB_GCC) || defined(EFLIB_CLANG)
  dll_name = "libsasl_host";
#endif  // defined

#ifdef EFLIB_DEBUG
  dll_name += "_d";
#endif
#if defined(EFLIB_WINDOWS)
  dll_name += ".dll";
#elif defined(EFLIB_LINUX) || defined(EFLIB_MACOS)
  dll_name += ".so";
#else
#  error "Unknown system."
#endif

  host_lib = dynamic_lib::load(dll_name);
  host_lib->get_function(compile_func, "salvia_compile_shader");
  host_lib->get_function(compile_from_file_func, "salvia_compile_shader_file");
  host_lib->get_function(create_host_func, "salvia_create_host");
}

void host::compile(shader_object_ptr& obj,
                   shader_log_ptr& log,
                   string const& code,
                   shader_profile const& prof,
                   vector<external_function_desc> const& funcs) {
  if (!compile_func) {
    load_function();
  }
  assert(compile_func);

  if (!compile_func)
    return;
  compile_func(obj, log, code, prof, funcs);
}

void host::compile_from_file(shader_object_ptr& obj,
                             shader_log_ptr& log,
                             string const& file_name,
                             shader_profile const& prof,
                             vector<external_function_desc> const& funcs) {
  if (!compile_from_file_func) {
    load_function();
  }

  if (!compile_from_file_func)
    return;
  compile_from_file_func(obj, log, file_name, prof, funcs);
}

host_ptr host::create_host() {
  host_ptr ret;

  if (!create_host_func) {
    load_function();
  }

  if (create_host_func) {
    create_host_func(ret);
  }

  return ret;
}
}  // namespace modules
}  // namespace salvia::core
