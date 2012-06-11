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
		class module_semantic;
		class abi_info;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_module: public codegen_context{
public:
	virtual llvm::Module*		module() const					= 0;
	virtual llvm::Module*		get_ownership() const			= 0;
	virtual llvm::LLVMContext&	context()						= 0;
	virtual void				dump() const					= 0;
	virtual void				dump( std::ostream& ostr ) const= 0;

	virtual ~llvm_module(){};
};

enum optimization_options{
	opt_verify,
	opt_preset_std_for_function
};

boost::shared_ptr<llvm_module> generate_llvm_code( sasl::semantic::module_semantic*, sasl::semantic::abi_info const* );
void optimize( boost::shared_ptr<llvm_module>, std::vector<optimization_options> opt_options );

// void optimize( boost::shared_ptr<llvm_module>, const std::string& params );

END_NS_SASL_CODE_GENERATOR();

#endif