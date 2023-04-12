#include <sasl/codegen/ty_cache.h>

#include <sasl/enums/traits.h>

#include <eflib/diagnostics/assert.h>

#include <fmt/format.h>

#if __has_include(<boost/unordered/unordered_flat_map.hpp>)
#  include <boost/unordered/unordered_flat_map.hpp>
#  define fast_unordered_map boost::unordered::unordered_flat_map
#else
#  include <unordered_map>
#  define fast_unordered_map std::unordered_map
#endif

#include <vector>

using namespace sasl::enums;
using std::vector;

namespace sasl::codegen {
using eflib::e2i;

class ty_cache_t {
public:
  Type* type(LLVMContext& ctxt, builtin_types bt, abis abi);
  std::string const& name(builtin_types bt, abis abi);
  void initialize(LLVMContext& ctxt);

private:
  Type* create_ty(LLVMContext& ctxt, builtin_types bt, abis abi);
  Type* create_abi_ty(LLVMContext& ctxt, builtin_types bt, abis abi);

  fast_unordered_map<LLVMContext*, fast_unordered_map<builtin_types, Type*>>
      cache[e2i(abis::count)];
  fast_unordered_map<builtin_types, std::string> ty_name[e2i(abis::count)];
};

Type* ty_cache_t::type(LLVMContext& ctxt, builtin_types bt, abis abi) {
  if (abi == abis::unknown) {
    return nullptr;
  }

  auto& ty_table = cache[static_cast<int>(abi)][&ctxt];
  auto ty_table_it = ty_table.find(bt);

  if (ty_table_it != ty_table.end()) {
    return ty_table_it->second;
  }

  Type*& found_ty = ty_table[bt];
  found_ty = create_ty(ctxt, bt, abi);
  if (is_sampler(bt)) {
    cache[static_cast<int>(abis::c)][&ctxt][bt] = cache[static_cast<int>(abis::llvm)][&ctxt][bt] =
        found_ty;
  }

  return found_ty;
}

std::string const& ty_cache_t::name(builtin_types bt, abis abi) {
  std::string& ret_name = ty_name[static_cast<int>(abi)][bt];
  char const* suffix = nullptr;
  switch (abi) {
  case abis::c: suffix = ".c"; break;
  case abis::llvm: suffix = ".l"; break;
  case abis::regs: suffix = ".r"; break;
  default: assert(false); break;
  }

  if (ret_name.empty()) {
    if (is_scalar(bt)) {
      if (bt == builtin_types::_void) {
        ret_name = "void";
      } else if (bt == builtin_types::_sint8) {
        ret_name = "char";
      } else if (bt == builtin_types::_sint16) {
        ret_name = "short";
      } else if (bt == builtin_types::_sint32) {
        ret_name = "int";
      } else if (bt == builtin_types::_sint64) {
        ret_name = "int64";
      } else if (bt == builtin_types::_uint8) {
        ret_name = "uchar";
      } else if (bt == builtin_types::_uint16) {
        ret_name = "ushort";
      } else if (bt == builtin_types::_uint32) {
        ret_name = "uint";
      } else if (bt == builtin_types::_uint64) {
        ret_name = "uint64";
      } else if (bt == builtin_types::_float) {
        ret_name = "float";
      } else if (bt == builtin_types::_double) {
        ret_name = "double";
      } else if (bt == builtin_types::_boolean) {
        ret_name = "bool";
      }
    } else if (is_vector(bt)) {
      ret_name = fmt::format("{:s}.v{:d}{:s}", name(scalar_of(bt), abi), vector_size(bt), suffix);
    } else if (is_matrix(bt)) {
      ret_name = fmt::format("{:s}.m{:d}{:d}{:s}",
                             name(scalar_of(bt), abi),
                             vector_size(bt),
                             vector_count(bt),
                             suffix);
    }
  }
  return ret_name;
}

Type* ty_cache_t::create_ty(LLVMContext& ctxt, builtin_types bt, abis abi) {
  if (is_void(bt)) {
    return Type::getVoidTy(ctxt);
  }

  if (bt == builtin_types::_sampler) {
    return Type::getInt8PtrTy(ctxt);
  }

  switch (abi) {
  case abis::c:
  case abis::llvm:
  case abis::regs: return create_abi_ty(ctxt, bt, abi);
  default: assert(false); return nullptr;
  }
}

void ty_cache_t::initialize(LLVMContext& ctxt) {
  cache[static_cast<int>(abis::c)].erase(&ctxt);
  cache[static_cast<int>(abis::llvm)].erase(&ctxt);
}

Type* ty_cache_t::create_abi_ty(LLVMContext& ctxt, builtin_types bt, abis abi) {
  if (is_scalar(bt)) {
    if (bt == builtin_types::_boolean) {
      return IntegerType::get(ctxt, 8);
    }
    if (is_integer(bt)) {
      return IntegerType::get(ctxt, (unsigned int)storage_size(bt) << 3);
    }
    if (bt == builtin_types::_float) {
      return Type::getFloatTy(ctxt);
    }
    if (bt == builtin_types::_double) {
      return Type::getDoubleTy(ctxt);
    }
  }

  if (is_vector(bt)) {
    Type* elem_ty = type(ctxt, scalar_of(bt), abi);
    size_t vec_size = vector_size(bt);
    switch (abi) {
    case abis::c: {
      vector<Type*> elem_tys(vec_size, elem_ty);
      return StructType::create(elem_tys, name(bt, abi), true);
    }
    case abis::llvm: return VectorType::get(elem_ty, static_cast<unsigned int>(vec_size), false);
    case abis::regs: return ArrayType::get(elem_ty, vec_size);
    default: return nullptr;
    }
  }

  if (is_matrix(bt)) {
    Type* vec_ty = type(ctxt, vector_of(scalar_of(bt), vector_size(bt)), abi);
    vector<Type*> row_tys(vector_count(bt), vec_ty);
    return StructType::create(row_tys, name(bt, abi), true);
  }

  return nullptr;
}

ty_cache_t cache;
Type* get_llvm_type(LLVMContext& ctxt, builtin_types bt, abis abi) {
  return cache.type(ctxt, bt, abi);
}

void initialize_cache(LLVMContext& ctxt) {
  cache.initialize(ctxt);
}

}  // namespace sasl::codegen
