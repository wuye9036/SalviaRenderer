#ifndef SASL_CODEGEN_CG_API_H
#define SASL_CODEGEN_CG_API_H

#include <sasl/include/codegen/forward.h>

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

BEGIN_NS_SASL_CODEGEN();

class module_context;

class cgllvm_module
{
public:
	virtual sasl::semantic::module_semantic*
								get_semantic() const				= 0;
	virtual sasl::codegen::module_context*
								get_context() const					= 0;

	virtual llvm::Module*		llvm_module() const					= 0;
	virtual llvm::LLVMContext&	llvm_context()						= 0;
	virtual llvm::Module*		take_ownership() const				= 0;

	virtual void				dump_ir() const						= 0;
	virtual void				dump_ir( std::ostream& ostr ) const	= 0;

	virtual ~cgllvm_module(){};
};

enum optimization_options{
	opt_verify,
	opt_preset_std_for_function
};

boost::shared_ptr<cgllvm_module> generate_llvm_code(
	boost::shared_ptr<sasl::semantic::module_semantic> const&,
	sasl::semantic::abi_info const*
	);
void optimize( boost::shared_ptr<cgllvm_module>, std::vector<optimization_options> opt_options );

// void optimize( boost::shared_ptr<llvm_module>, const std::string& params );

END_NS_SASL_CODEGEN();

#endif