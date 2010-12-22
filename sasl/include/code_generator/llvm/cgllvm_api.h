#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_API_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_API_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/codegen_context.h>
#include <boost/shared_ptr.hpp>

namespace llvm {
	class Module;
	class LLVMContext;
}

namespace sasl {
	namespace semantic{
		class symbol;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_code: public codegen_context{
public:
	virtual llvm::Module* module() const = 0;
	virtual llvm::Module* get_ownership() const = 0;
	virtual llvm::LLVMContext& context() = 0;
	virtual ~llvm_code(){};
};

boost::shared_ptr<llvm_code> generate_llvm_code( boost::shared_ptr<sasl::semantic::symbol> root );
END_NS_SASL_CODE_GENERATOR();

#endif