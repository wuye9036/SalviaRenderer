#pragma once

#include <eflib/utility/shared_declaration.h>

#include <functional>
#include <string>
#include <tuple>
#include <vector>

namespace salvia::shader {
struct external_function_desc;
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_reflection2);
}  // namespace salvia::shader

namespace sasl::common {
EFLIB_DECLARE_CLASS_SHARED_PTR(diag_chat);
EFLIB_DECLARE_CLASS_SHARED_PTR(code_source);
EFLIB_DECLARE_CLASS_SHARED_PTR(lex_context);
}  // namespace sasl::common

namespace sasl::semantic {
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
EFLIB_DECLARE_CLASS_SHARED_PTR(reflection_impl);
}  // namespace sasl::semantic

namespace sasl::codegen {
EFLIB_DECLARE_CLASS_SHARED_PTR(module_vmcode);
}

namespace sasl::syntax_tree {
EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
}

namespace sasl::drivers {

typedef std::function<bool /*succeed*/ (std::string& /*[out]content*/,
                                        std::string& /*[out]native file name*/,
                                        std::string const& /*file name*/,
                                        bool /*is system header*/,
                                        bool /*check only*/)>
    include_handler_fn;

class compiler {
public:
  virtual void set_parameter(int argc, char** argv) = 0;
  virtual void set_parameter(std::string const& cmd) = 0;

  virtual void set_code_source(sasl::common::code_source_ptr const&) = 0;
  virtual void set_code(std::string const& code_text) = 0;
  virtual void set_code_file(std::string const& code_file) = 0;

  virtual void add_virtual_file(std::string const& file_name,
                                std::string const& code_content,
                                bool high_priority) = 0;
  virtual void set_include_handler(include_handler_fn inc_handler) = 0;

  virtual sasl::common::diag_chat_ptr compile(bool enable_reflect2) = 0;
  virtual sasl::common::diag_chat_ptr
  compile(std::vector<salvia::shader::external_function_desc> const&, bool enable_reflect2) = 0;

  virtual sasl::semantic::module_semantic_ptr get_semantic() const = 0;
  virtual sasl::codegen::module_vmcode_ptr get_vmcode() const = 0;
  virtual sasl::syntax_tree::node_ptr get_root() const = 0;
  virtual sasl::semantic::reflection_impl_ptr get_reflection() const = 0;
  virtual salvia::shader::shader_reflection2_ptr get_reflection2() const = 0;

  virtual ~compiler() = default;
};

}  // namespace sasl::drivers