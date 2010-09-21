#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_JIT_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_JIT_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/jit_api.h>
namespace llvm{ class Function; }

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_global_context;
class cgllvm_common_context;

void* cgllvm_jit( llvm::Function* func );

template <typename FunctionT>
FunctionT* cgllvm_jit( const cgllvm_global_context& /*ctxt*/, const std::string& func_name );
template <typename FunctionT>
FunctionT* cgllvm_jit( FunctionT*& out, const cgllvm_global_context& /*ctxt*/, const std::string& func_name );
template <typename FunctionT>
FunctionT* cgllvm_jit( const cgllvm_common_context& /*ctxt*/ );
template <typename FunctionT>
FunctionT* cgllvm_jit( FunctionT*& out, const cgllvm_common_context& /*ctxt*/ );

END_NS_SASL_CODE_GENERATOR();

#endif