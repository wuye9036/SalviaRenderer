#ifndef SASL_CODEGEN_TY_CACHE_H
#define SASL_CODEGEN_TY_CACHE_H

#include <sasl/include/codegen/forward.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/include/codegen/cgs.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>
#include <vector>

BEGIN_NS_SASL_CODEGEN();

using namespace llvm;
using boost::unordered_map;
using boost::lexical_cast;
using std::string;
using std::vector;

void initialize_cache( LLVMContext& ctxt );
Type* get_llvm_type( LLVMContext& ctxt, builtin_types bt, abis abi );

END_NS_SASL_CODEGEN();

#endif