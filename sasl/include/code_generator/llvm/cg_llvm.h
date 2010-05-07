#ifndef SASL_CODE_GENERATOR_LLVM_CG_LLVM_H
#define SASL_CODE_GENERATOR_LLVM_CG_LLVM_H

#include <sasl/include/code_generator/forward.h>
#include <boost/shared_ptr.hpp>

namespace llvm {
	class Module;
}

namespace sasl {
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();
boost::shared_ptr<llvm::Module> generate_llvm_code( boost::shared_ptr<sasl::syntax_tree::node> root );
END_NS_SASL_CODE_GENERATOR();

#endif