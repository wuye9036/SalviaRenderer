#pragma once

#include <sasl/drivers/compiler.h>
#include <sasl/drivers/options.h>

namespace salvia::shader {
struct external_function_desc;
}

namespace sasl::drivers {

class compiler_impl : public compiler {
public:
  compiler_impl();

  // All setting functions must be called before calling compile().
  void set_parameter(int argc, char** argv) override;
  void set_parameter(std::string const& cmd) override;

  void set_code(std::string const& code_text) override;
  void set_code_file(std::string const& code_file) override;
  void set_code_source(std::shared_ptr<sasl::common::code_source> const&) override;
  virtual void set_lex_context(std::shared_ptr<sasl::common::lex_context> const&);

  /// Only support by default code_source.
  void add_virtual_file(std::string const& file_name,
                        std::string const& code_content,
                        bool high_priority) override;
  /// Only support by default code_source.
  void set_include_handler(include_handler_fn inc_handler) override;
  /// Only support by default code source.
  virtual void add_include_path(std::string const& inc_path);
  /// Only support by default code source.
  virtual void add_sys_include_path(std::string const& sys_path);
  /// Only support by default code source.
  virtual void add_macro(std::string const& macro, bool predef);
  /// Only support by default code source.
  virtual void remove_macro(std::string const& macro);
  /// Only support by default code source.

  sasl::common::diag_chat_ptr compile(bool enable_reflect2) override;
  sasl::common::diag_chat_ptr compile(std::vector<salvia::shader::external_function_desc> const&,
                                      bool enable_reflect2) override;

  sasl::semantic::module_semantic_ptr get_semantic() const override;
  sasl::codegen::module_vmcode_ptr get_vmcode() const override;
  sasl::syntax_tree::node_ptr get_root() const override;
  sasl::semantic::reflection_impl_ptr get_reflection() const override;
  salvia::shader::shader_reflection2_ptr get_reflection2() const override;

  boost::program_options::variables_map const& variables() const;
  options_display_info const& display_info() const;
  options_io const& io_info() const;

private:
  compiler_impl(compiler_impl const&);
  compiler_impl& operator=(compiler_impl const&);

  template <typename ParserT>
  bool parse(ParserT& parser);

  void inject_default_functions();

  void inject_function(void* pfn, std::string_view name, bool is_raw_name);

  sasl::semantic::module_semantic_ptr mod_sem_;
  sasl::codegen::module_vmcode_ptr mod_vm_code_;
  sasl::syntax_tree::node_ptr root_;
  sasl::semantic::reflection_impl_ptr reflection_;
  salvia::shader::shader_reflection2_ptr reflection2_;

  // Options
  options_global opt_global;
  options_display_info opt_disp;
  options_io opt_io;
  option_macros opt_macros;
  options_includes opt_includes;

  po::options_description desc;
  po::variables_map vm;

  // Overridden options
  std::shared_ptr<sasl::common::code_source> user_code_src;
  std::shared_ptr<sasl::common::lex_context> user_lex_ctxt;

  typedef std::unordered_map<std::string, std::pair<std::string, bool>> virtual_file_dict;
  enum macro_states { ms_normal, ms_predef, ms_remove };
  std::vector<std::string> sys_paths, inc_paths;
  std::vector<std::pair<std::string, macro_states>> macros;
  include_handler_fn user_inc_handler;
  virtual_file_dict virtual_files;
};

}  // namespace sasl::drivers
