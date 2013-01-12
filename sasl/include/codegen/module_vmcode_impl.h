#ifndef SASL_CODEGEN_CG_MODULE_IMPL_H
#define SASL_CODEGEN_CG_MODULE_IMPL_H

#include <sasl/include/codegen/forward.h>
#include <sasl/include/codegen/cg_api.h>
#include <sasl/include/semantic/reflection_impl.h>

#include <eflib/include/string/ustring.h>

namespace llvm
{
	class LLVMContext;
	class Module;
	class ConstantFolder;
	class ExecutionEngine;
	class Function;
	class Type;
	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
        > class IRBuilder;
    typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
        DefaultIRBuilder;
}

namespace sasl
{
	namespace semantic
	{
		EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
	}
}

#include <boost/shared_ptr.hpp>
#include <string>

BEGIN_NS_SASL_CODEGEN();

EFLIB_DECLARE_CLASS_SHARED_PTR(module_context);

// module_vmcode_impl contains all LLVM related objects which are used by JIT.
class module_vmcode_impl: public module_vmcode{
public:
	module_vmcode_impl(eflib::fixed_string const& module_name);

	virtual sasl::semantic::module_semantic*
									get_semantic() const;
	virtual void					set_semantic( sasl::semantic::module_semantic_ptr const& );
	virtual module_context*			get_context() const;
	virtual void					set_context( module_context_ptr const& );

	virtual bool					enable_jit();
	virtual void*					get_function	(eflib::fixed_string const&);
	virtual void					inject_function	(void* , eflib::fixed_string const&);

	virtual llvm::Module*			get_vm_module() const;
	virtual llvm::LLVMContext&		get_vm_context();
	virtual llvm::DefaultIRBuilder*	builder() const;

	virtual void dump_ir() const;
	virtual void dump_ir( std::ostream& ostr ) const;

	~module_vmcode_impl();

protected:
	sasl::semantic::module_semantic_ptr	sem_;

	module_context_ptr					ctxt_;

	llvm::LLVMContext*					vm_ctx_;
	llvm::DefaultIRBuilder*				irbuilder_;
	llvm::Module*						vm_module_;
	llvm::ExecutionEngine*				vm_engine_;
	eflib::fixed_string					error_;

	std::vector<llvm::Function*>		jitted_funcs_;
};

END_NS_SASL_CODEGEN();

#endif
