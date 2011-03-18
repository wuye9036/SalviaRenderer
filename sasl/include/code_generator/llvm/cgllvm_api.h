#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_API_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_API_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/codegen_context.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <ostream>
#include <string>
#include <vector>

namespace llvm {
	class Module;
	class LLVMContext;
}

namespace sasl {
	namespace semantic{
		class module_si;
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

enum optimization_options{
	opt_verify,
	opt_preset_std_for_function
};

boost::shared_ptr<llvm_code> generate_llvm_code( boost::shared_ptr< sasl::semantic::module_si > );
void optimize( boost::shared_ptr<llvm_code>, std::vector<optimization_options> opt_options );
void dump( boost::shared_ptr<llvm_code> );
void dump( boost::shared_ptr<llvm_code>, std::ostream& o );
// void optimize( boost::shared_ptr<llvm_code>, const std::string& params );

END_NS_SASL_CODE_GENERATOR();

#endif