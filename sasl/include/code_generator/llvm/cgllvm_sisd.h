#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SISD_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SISD_H

#include <sasl/include/code_generator/llvm/cgllvm_impl.h>

namespace llvm{
	class Constant;
}

BEGIN_NS_SASL_CODE_GENERATOR();

// Code generation for SISD( Single Instruction Single Data )
class cgllvm_sisd: public cgllvm_impl{

protected:
	llvm::Constant* zero_value( boost::shared_ptr<sasl::syntax_tree::type_specifier> );
};

END_NS_SASL_CODE_GENERATOR();

#endif