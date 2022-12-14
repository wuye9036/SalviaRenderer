#pragma once

#include <sasl/semantic/semantic_forward.h>
#include <sasl/enums/builtin_types.h>
#include <salvia/shader/reflection.h>
#include <eflib/utility/shared_declaration.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace sasl {
namespace syntax_tree {
EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
}
} // namespace sasl

namespace sasl::semantic {

EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);

//////////////////////////////////////////////////////////////////////////
// Application binary interface information.
// Used by host and interpolator / rasterizer.
class reflector;

class reflection_impl : public salvia::shader::shader_reflection {
public:
  // Friend for reflector could call compute_layout();
  friend class reflector;

  // Implements members of shader_reflection
  virtual salvia::shader::languages get_language() const;
  virtual std::string_view entry_name() const;
  virtual std::vector<salvia::shader::sv_layout *> layouts(salvia::shader::sv_usage usage) const;
  virtual size_t layouts_count(salvia::shader::sv_usage usage) const;
  virtual size_t total_size(salvia::shader::sv_usage usage) const;

  virtual salvia::shader::sv_layout *input_sv_layout(std::string_view) const;
  virtual salvia::shader::sv_layout *output_sv_layout(salvia::shader::semantic_value const &) const;

  virtual bool has_position_output() const;

  // Impl specific members
  reflection_impl();

  void update_size(size_t sz, salvia::shader::sv_usage usage);

  void module(module_semantic_ptr const &);
  bool is_module(module_semantic_ptr const &) const;

  void entry(symbol *);
  bool is_entry(symbol *) const;

  bool add_input_semantic(salvia::shader::semantic_value const &sem, builtin_types btc,
                          bool is_stream);
  bool add_output_semantic(salvia::shader::semantic_value const &sem, builtin_types btc,
                           bool is_stream);
  void add_global_var(symbol *, sasl::syntax_tree::tynode_ptr btc);

  salvia::shader::sv_layout *input_sv_layout(salvia::shader::semantic_value const &) const;
  salvia::shader::sv_layout *input_sv_layout(symbol *) const;

private:
  salvia::shader::sv_layout *alloc_input_layout(salvia::shader::semantic_value const &);
  salvia::shader::sv_layout *alloc_input_layout(symbol *);
  salvia::shader::sv_layout *alloc_output_layout(salvia::shader::semantic_value const &);

  module_semantic *module_sem_;
  symbol *entry_point_;
  std::string_view entry_point_name_;
  salvia::shader::sv_layout *position_output_;

  // Include su_stream_in and su_buffer_in

  std::unordered_map<salvia::shader::semantic_value, salvia::shader::sv_layout,
                     std::hash<salvia::shader::semantic_value>>
      semantic_input_layouts_;
  std::unordered_map<salvia::shader::semantic_value, salvia::shader::sv_layout,
                     std::hash<salvia::shader::semantic_value>>
      semantic_output_layouts_;
  std::unordered_map<symbol *, salvia::shader::sv_layout *> uniform_input_layouts_;

  std::vector<symbol *> uniform_inputs_;
  std::vector<salvia::shader::semantic_value> semantic_inputs_;
  std::vector<salvia::shader::semantic_value> semantic_outputs_;

  std::unordered_map<std::string_view, salvia::shader::sv_layout *> name_layouts_;

  // The count and offsets of sv_usages
  size_t counts_[salvia::shader::sv_usage_count];
  size_t offsets_[salvia::shader::sv_usage_count];
};

} // namespace sasl::semantic
