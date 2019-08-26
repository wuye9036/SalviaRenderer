#pragma once

#include <sasl/include/codegen/forward.h>
#include <sasl/include/codegen/cg_api.h>
#include <sasl/include/semantic/reflection_impl.h>

#include <eflib/include/string/ustring.h>

namespace llvm
{
	class LLVMContext;
	class Module;
	class ExecutionEngine;
	class Function;
	class Type;

    class IRBuilderDefaultInserter;
    template <typename T, typename Inserter> class IRBuilder;
    class ConstantFolder;
    using DefaultIRBuilder = IRBuilder<ConstantFolder, IRBuilderDefaultInserter>;
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

	virtual void*					get_function	(eflib::fixed_string const&);
	virtual void					inject_function	(void* , eflib::fixed_string const&);

	virtual llvm::Module*			get_vm_module() const;
	virtual llvm::LLVMContext&		get_vm_context();
	virtual llvm::DefaultIRBuilder*	builder() const;

	virtual void dump_ir() const;
	virtual void dump_ir( std::ostream& ostr ) const;

	~module_vmcode_impl();

protected:
	std::unique_ptr<llvm::LLVMContext>		vm_ctx_;
	std::unique_ptr<llvm::ExecutionEngine>  vm_engine_;
	std::unique_ptr<llvm::DefaultIRBuilder> ir_builder_;
	llvm::Module*							vm_module_;

	sasl::semantic::module_semantic_ptr	sem_;
	module_context_ptr					ctxt_;
	eflib::fixed_string					error_;
	bool								finalized_;
};

END_NS_SASL_CODEGEN();
