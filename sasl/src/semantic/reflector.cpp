#include <sasl/semantic/reflector.h>

#include <sasl/common/diag_chat.h>
#include <sasl/semantic/reflection_impl.h>
#include <sasl/semantic/semantic_diags.h>
#include <sasl/semantic/semantics.h>
#include <sasl/semantic/symbol.h>
#include <sasl/syntax_tree/declaration.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/traits.h>

#include <algorithm>
#include <eflib/diagnostics/assert.h>
#include <salvia/shader/constants.h>

using namespace sasl::syntax_tree;
using namespace sasl::enums;
using namespace sasl::common;

using salvia::shader::languages;
using salvia::shader::su_buffer_in;
using salvia::shader::su_buffer_out;
using salvia::shader::su_none;
using salvia::shader::su_stream_in;
using salvia::shader::su_stream_out;
using salvia::shader::sv_usage;
using salvia::shader::sv_usage_count;

using std::addressof;
using std::make_shared;
using std::shared_ptr;

using std::lower_bound;
using std::string;
using std::vector;

namespace sasl::semantic {

bool verify_semantic_type(builtin_types btc, salvia::shader::semantic_value const &sem) {
  switch (sem.get_system_value()) {

  case salvia::shader::sv_none:
    return false;

  case salvia::shader::sv_position:
  case salvia::shader::sv_texcoord:
  case salvia::shader::sv_normal:
  case salvia::shader::sv_target:
    return (is_scalar(btc) || is_vector(btc) || is_matrix(btc)) &&
           (scalar_of(btc) == builtin_types::_float || scalar_of(btc) == builtin_types::_sint32);
  case salvia::shader::sv_blend_indices:
    return (is_scalar(btc) || is_vector(btc)) && is_integer(scalar_of(btc));
  case salvia::shader::sv_blend_weights:
    return (is_scalar(btc) || is_vector(btc)) && (scalar_of(btc) == builtin_types::_float);
  case salvia::shader::sv_depth:
    return (btc == builtin_types::_float);
  default:
    EFLIB_ASSERT_UNIMPLEMENTED();
  }

  return false;
}

sv_usage vsinput_semantic_usage(salvia::shader::semantic_value const &sem) {
  switch (sem.get_system_value()) {
  case salvia::shader::sv_position:
  case salvia::shader::sv_texcoord:
  case salvia::shader::sv_normal:
  case salvia::shader::sv_blend_indices:
  case salvia::shader::sv_blend_weights:
    return su_stream_in;
  }
  EFLIB_ASSERT_UNIMPLEMENTED();
  return su_none;
}

sv_usage vsoutput_semantic_usage(salvia::shader::semantic_value const &sem) {
  switch (sem.get_system_value()) {
  case salvia::shader::sv_position:
    return su_buffer_out;
  case salvia::shader::sv_texcoord:
    return su_buffer_out;
  }
  EFLIB_ASSERT_UNIMPLEMENTED();
  return su_none;
}

sv_usage psinput_semantic_usage(salvia::shader::semantic_value const &sem) {
  switch (sem.get_system_value()) {
  case salvia::shader::sv_texcoord:
    return su_stream_in;
  }
  EFLIB_ASSERT_UNIMPLEMENTED();
  return su_none;
}

sv_usage psoutput_semantic_usage(salvia::shader::semantic_value const &sem) {
  switch (sem.get_system_value()) {
  case salvia::shader::sv_target:
    return su_stream_out;
  }
  EFLIB_ASSERT_UNIMPLEMENTED();
  return su_none;
}

sv_usage semantic_usage(salvia::shader::languages lang, bool is_output,
                        salvia::shader::semantic_value const &sem) {
  switch (lang) {
  case salvia::shader::lang_vertex_shader:
    if (is_output) {
      return vsoutput_semantic_usage(sem);
    } else {
      return vsinput_semantic_usage(sem);
    }
  case salvia::shader::lang_pixel_shader:
    if (is_output) {
      return psoutput_semantic_usage(sem);
    } else {
      return psinput_semantic_usage(sem);
    }
  }

  return su_none;
}

class reflector {
public:
  reflector(module_semantic *sem, std::string_view entry_name, diag_chat *diags)
      : sem_(sem), current_entry_(nullptr), reflection_(nullptr), entry_name_(entry_name),
        diags_(diags) {}

  reflector(module_semantic *sem, diag_chat *diags)
      : sem_(sem), current_entry_(nullptr), reflection_(nullptr), diags_(diags) {}

  reflection_impl_ptr reflect() {
    if (!entry_name_.empty()) {
      vector<symbol *> overloads = sem_->root_symbol()->find_overloads(entry_name_);
      if (overloads.size() != 1) {
        return reflection_impl_ptr();
      }
      current_entry_ = overloads[0];
      return do_reflect();
    } else {
      symbol *candidate = nullptr;
      reflection_impl_ptr candidate_reflection;
      for (symbol *fn_sym : sem_->functions()) {
        current_entry_ = fn_sym;
        candidate_reflection = do_reflect();

        if (candidate_reflection) {
          if (candidate) {
            // TODO: More than one matched. conflict error.
            return reflection_impl_ptr();
          }
          candidate = fn_sym;
        }
      }
      return candidate_reflection;
    }
  }

private:
  reflection_impl_ptr do_reflect() {
    if (!(sem_ && current_entry_)) {
      return reflection_impl_ptr();
    }

    salvia::shader::languages lang = sem_->get_language();

    // Initialize language ABI information.
    reflection_impl_ptr ret = make_shared<reflection_impl>();
    ret->module_sem_ = sem_;
    ret->entry_point_ = current_entry_;
    ret->entry_point_name_ = current_entry_->mangled_name();
    reflection_ = ret.get();

    if (lang == salvia::shader::lang_vertex_shader || lang == salvia::shader::lang_pixel_shader ||
        lang == salvia::shader::lang_blending_shader) {
      // Process entry function.
      shared_ptr<function_def> entry_fn =
          current_entry_->associated_node()->as_handle<function_def>();
      assert(entry_fn);

      if (!add_semantic(entry_fn, false, false, false, lang, true)) {
        assert(false);
        ret.reset();
        return ret;
      }

      for (shared_ptr<parameter> const &param : entry_fn->params) {
        if (!add_semantic(param, false, false, false, lang, false)) {
          ret.reset();
          return ret;
        }
      }

      // Process global variables.
      for (symbol *gvar_sym : sem_->global_vars()) {
        shared_ptr<declarator> gvar = gvar_sym->associated_node()->as_handle<declarator>();
        assert(gvar);

        // And global variable only be treated as input.
        if (!add_semantic(gvar, true, false, false, lang, false)) {
          // If it is not attached to an valid semantic, it should be uniform variable.

          // Check the data type of global. Now global variables only support built-in types.
          node_semantic *psi = sem_->get_semantic(gvar.get());
          if (psi->ty_proto()->is_builtin() || psi->ty_proto()->is_array()) {
            ret->add_global_var(gvar_sym, psi->ty_proto()->as_handle<tynode>());
          } else {
            // TODO: It an semantic error need to be reported.
            ret.reset();
            return ret;
          }
        }
      }
    }

    return ret;
  }

  bool add_semantic(node_ptr const &v, bool is_global, bool is_member, bool enable_nested,
                    languages lang, bool is_output_semantic) {
    assert(reflection_);
    node_semantic *pssi = sem_->get_semantic(v.get());
    assert(pssi); // TODO: Here are semantic analysis error.
    tynode *ptspec = pssi->ty_proto();
    assert(ptspec); // TODO: Here are semantic analysis error.

    salvia::shader::semantic_value const &node_sem = pssi->semantic_value_ref();

    if (ptspec->is_builtin()) {
      builtin_types btc = ptspec->tycode;
      if (verify_semantic_type(btc, node_sem)) {
        sv_usage sem_s = semantic_usage(lang, is_output_semantic, node_sem);
        switch (sem_s) {
        case su_stream_in:
          return reflection_->add_input_semantic(node_sem, btc, true);
        case su_buffer_in:
          return reflection_->add_input_semantic(node_sem, btc, false);
        case su_stream_out:
          return reflection_->add_output_semantic(node_sem, btc, true);
        case su_buffer_out:
          return reflection_->add_output_semantic(node_sem, btc, false);
        }

        assert(false);
        return false;
      } else if (is_member) {
        diags_->report(not_support_auto_semantic, "", code_span{});
        return false;
      }
    } else if (ptspec->node_class() == node_ids::struct_type) {
      if ((is_member || is_global) && !enable_nested) {
        return false;
      }

      // TODO: do not support nested aggregated variable.
      struct_type *pstructspec = dynamic_cast<struct_type *>(ptspec);
      assert(pstructspec);
      for (shared_ptr<declaration> const &decl : pstructspec->decls) {
        if (decl->node_class() == node_ids::variable_declaration) {
          shared_ptr<variable_declaration> vardecl = decl->as_handle<variable_declaration>();
          for (shared_ptr<declarator> const &dclr : vardecl->declarators) {
            if (!add_semantic(dclr, is_global, true, enable_nested, lang, is_output_semantic)) {
              return false;
            }
          }
        }
      }

      return true;
    }

    return false;
  }

  diag_chat *diags_;
  module_semantic *sem_;
  std::string_view entry_name_;
  symbol *current_entry_;
  reflection_impl *reflection_;
};

reflection_impl_ptr reflect(module_semantic_ptr const &sem, diag_chat *diags) {
  reflector rfl(sem.get(), diags);
  return rfl.reflect();
}

reflection_impl_ptr reflect(module_semantic_ptr const &sem, std::string_view entry_name,
                            diag_chat *diags) {
  reflector rfl(sem.get(), entry_name, diags);
  return rfl.reflect();
}

} // namespace sasl::semantic