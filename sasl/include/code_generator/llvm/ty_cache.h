#ifndef SASL_CODE_GENERATOR_LLVM_TY_CACHE_H
#define SASL_CODE_GENERATOR_LLVM_TY_CACHE_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/include/code_generator/llvm/cgllvm_service.h>

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

BEGIN_NS_SASL_CODE_GENERATOR();

using namespace llvm;
using boost::unordered_map;
using boost::lexical_cast;
using std::string;
using std::vector;

class ty_cache_t{
public:
	Type* type( LLVMContext& ctxt, builtin_types bt, abis abi );
	std::string const& name( builtin_types bt, abis abi );
private:
	Type* create_ty( LLVMContext& ctxt, builtin_types bt, abis abi );

	unordered_map<LLVMContext*, unordered_map<builtin_types, Type*> > cache[2];
	unordered_map<builtin_types, std::string> ty_name[2];
};

Type* get_llvm_type( LLVMContext& ctxt, builtin_types bt, abis abi );

END_NS_SASL_CODE_GENERATOR();

#endif