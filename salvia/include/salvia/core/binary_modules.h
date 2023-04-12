#pragma once

#include <eflib/utility/shared_declaration.h>

#include <memory>
#include <string>
#include <vector>

namespace salvia::shader {
struct external_function_desc;
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
struct shader_profile;
}  // namespace salvia::shader

namespace salvia::core {

EFLIB_DECLARE_CLASS_SHARED_PTR(host);

namespace modules {
class host {
public:
  static void compile(shader::shader_object_ptr& obj,
                      shader::shader_log_ptr& log,
                      std::string const& code,
                      shader::shader_profile const& prof,
                      std::vector<shader::external_function_desc> const& funcs);
  static void compile_from_file(shader::shader_object_ptr& obj,
                                shader::shader_log_ptr& log,
                                std::string const& file_name,
                                shader::shader_profile const& prof,
                                std::vector<shader::external_function_desc> const& funcs);
  static host_ptr create_host();
};

}  // namespace modules

}  // namespace salvia::core
