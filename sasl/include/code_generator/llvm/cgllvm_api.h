#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_API_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_API_H

#include <sasl/include/code_generator/forward.h>
#include <boost/shared_ptr.hpp>

namespace llvm {
	class Module;
	class LLVMContext;
}

namespace sasl {
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_code{
public:
	virtual boost::shared_ptr<llvm::Module> module() const = 0;
	virtual llvm::LLVMContext& context() = 0;
	virtual ~llvm_code(){};
};

boost::shared_ptr<llvm_code> generate_llvm_code( boost::shared_ptr<sasl::syntax_tree::node> root );
END_NS_SASL_CODE_GENERATOR();

#endif