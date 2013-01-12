#ifndef SASL_CODEGEN_CG_API_H
#define SASL_CODEGEN_CG_API_H

#include <sasl/include/codegen/forward.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <ostream>
#include <string>
#include <vector>

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

BEGIN_NS_SASL_CODEGEN();

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

	virtual bool	enable_jit()			= 0;
	virtual void*	get_function	(eflib::fixed_string const& /*func_name*/)			= 0;
	virtual void	inject_function	(void* fn, eflib::fixed_string const& /*func_name*/)= 0;

	virtual void	dump_ir() const						= 0;
	virtual void	dump_ir( std::ostream& ostr ) const	= 0;

	virtual ~module_vmcode(){};
};

module_vmcode_ptr generate_vmcode(
	sasl::semantic::module_semantic_ptr const&,
	sasl::semantic::reflection_impl const*
	);

END_NS_SASL_CODEGEN();

#endif