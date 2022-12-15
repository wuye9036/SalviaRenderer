#ifndef SASL_CODEGEN_CG_API_H
#define SASL_CODEGEN_CG_API_H

#include <sasl/codegen/forward.h>

#include <eflib/utility/shared_declaration.h>

#include <ostream>
#include <string>
#include <vector>
#include <memory>

namespace llvm
{
	class Module;
	class LLVMContext;
}

namespace sasl
{
	namespace semantic
	{
		EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
		EFLIB_DECLARE_CLASS_SHARED_PTR(reflection_impl);
	}
}

namespace sasl::codegen {

class module_context;

EFLIB_DECLARE_CLASS_SHARED_PTR(module_vmcode);

class module_vmcode
{
public:
	virtual sasl::semantic::module_semantic*
					get_semantic() const	= 0;
	virtual sasl::codegen::module_context*
					get_context() const		= 0;

	virtual llvm::Module*
					get_vm_module() const	= 0;
	virtual llvm::LLVMContext&
					get_vm_context()		= 0;

	virtual void*	get_function	(std::string_view /*func_name*/)			= 0;
	virtual void	inject_function	(void* fn, std::string_view /*func_name*/)= 0;

	virtual void	dump_ir() const						= 0;
	virtual void	dump_ir( std::ostream& ostr ) const	= 0;

	virtual ~module_vmcode(){};
};

module_vmcode_ptr generate_vmcode(
	sasl::semantic::module_semantic_ptr const&,
	sasl::semantic::reflection_impl const*
	);

}

#endif