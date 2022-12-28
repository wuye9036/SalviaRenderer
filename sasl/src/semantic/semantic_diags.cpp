#include <sasl/semantic/semantic_diags.h>

#include <sasl/syntax_tree/declaration.h>

#include <eflib/diagnostics/assert.h>
#include <sasl/enums/traits.h>

#include <cassert>
#include <sstream>

using sasl::enums::is_matrix;
using sasl::enums::is_vector;
using sasl::enums::scalar_of;
using sasl::enums::vector_count;
using sasl::enums::vector_size;

using sasl::common::compiler_compatibility;
using sasl::common::diag_levels;
using sasl::common::diag_template;
using sasl::common::token;
using sasl::syntax_tree::function_full_def;
using sasl::syntax_tree::struct_type;
using sasl::syntax_tree::tynode;

using std::shared_ptr;

using std::string;
using std::stringstream;

namespace sasl::semantic {

char const *scalar_nick_name(builtin_types btcode) {
  if (btcode == builtin_types::_sint8) {
    return "char";
  } else if (btcode == builtin_types::_uint8) {
    return "byte";
  } else if (btcode == builtin_types::_sint16) {
    return "short";
  } else if (btcode == builtin_types::_uint16) {
    return "ushort";
  } else if (btcode == builtin_types::_sint32) {
    return "int";
  } else if (btcode == builtin_types::_uint32) {
    return "uint";
  } else if (btcode == builtin_types::_sint64) {
    return "long";
  } else if (btcode == builtin_types::_uint64) {
    return "ulong";
  } else if (btcode == builtin_types::_float) {
    return "float";
  } else if (btcode == builtin_types::_double) {
    return "double";
  } else if (btcode == builtin_types::none) {
    return "none";
  } else if (btcode == builtin_types::_sampler) {
    return "sampler";
  } else if (btcode == builtin_types::_boolean) {
    return "bool";
  } else {
    assert(false);
    return "<unknown>";
  }
}

type_repr::type_repr(shared_ptr<tynode> const &ty) : ty(ty) {}

type_repr::type_repr(sasl::syntax_tree::tynode *ty) { this->ty = ty->as_handle<tynode>(); }

string type_repr::str() {
  if (str_cache.empty()) {
    if (!ty) {
      str_cache = "<unknown>";
    } else if (ty->is_builtin()) {
      stringstream name_stream;

      builtin_types bt_code = ty->tycode;
      builtin_types scalar_code = scalar_of(bt_code);
      char const *scalar_name = scalar_nick_name(scalar_code);
      if (is_matrix(bt_code)) {
        name_stream << scalar_name << vector_count(bt_code) << "x" << vector_size(bt_code);
      } else if (is_vector(bt_code)) {
        name_stream << scalar_name << vector_size(bt_code);
      } else {
        name_stream << scalar_nick_name(bt_code);
      }
      std::string name = name_stream.str();
      str_cache.assign(name.begin(), name.end());
    } else if (ty->is_struct()) {
      str_cache = "struct ";
      str_cache += ty->as_handle<struct_type>()->name.lit();
    } else if (ty->is_function()) {
      shared_ptr<function_full_def> fn = ty->as_handle<function_full_def>();
      str_cache = type_repr(fn->retval_type).str();
      str_cache += " ";
      str_cache += fn->name.lit();
      str_cache += "(";
      if (!fn->params.empty()) {
        for (size_t i = 1; i < fn->params.size(); ++i) {
          str_cache += ", ";
          str_cache += type_repr(fn->params[i]->param_type).str();
        }
      }
      str_cache += ");";
    } else {
      ef_unimplemented();
    }
  }
  return str_cache;
}

args_type_repr &args_type_repr::arg(shared_ptr<tynode> const &arg_ty) {
  arg_tys.push_back(arg_ty);
  return *this;
}

args_type_repr &args_type_repr::arg(sasl::syntax_tree::node *arg_ty) {
  arg_tys.push_back(arg_ty->as_handle<tynode>());
  return *this;
}

string args_type_repr::str() {
  if (str_buffer.empty()) {
    stringstream sstr;

    if (arg_tys.empty()) {
      sstr << "( )";
    } else {
      sstr << "(" << type_repr(arg_tys[0]).str();
      for (size_t i = 1; i < arg_tys.size(); ++i) {
        sstr << ", " << type_repr(arg_tys[i]).str();
      }
      sstr << ")";
    }

    str_buffer = sstr.str();
  }

  return str_buffer;
}

args_type_repr::args_type_repr() {}

source_position_repr::source_position_repr(shared_ptr<token> const &beg,
                                           shared_ptr<token> const &end, compiler_compatibility cc)
    : beg(beg), end(end), cc(cc) {}

std::string source_position_repr::str() {
  ef_unimplemented();
  return std::string();
}

} // namespace sasl::semantic
