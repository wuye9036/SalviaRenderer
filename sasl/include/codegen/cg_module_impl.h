#ifndef SASL_CODEGEN_CG_MODULE_IMPL_H
#define SASL_CODEGEN_CG_MODULE_IMPL_H

#include <sasl/include/codegen/forward.h>
#include <sasl/include/codegen/cg_api.h>
#include <sasl/include/semantic/abi_info.h>

namespace llvm{
	class LLVMContext;
	class Module;
	class ConstantFolder;
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
		class module_semantic;
	}
}

#include <boost/shared_ptr.hpp>
#include <string>

BEGIN_NS_SASL_CODEGEN();

/////////////////////////////////////////
// support LLVMContext, Module, IRBuilder and other data use by code generator.
// General module created by llvm code generator.
class cg_module_impl: public cg_module{
public:
	cg_module_impl();
	void create_llvm_module(std::string const& module_name);

	virtual sasl::semantic::module_semantic*
									get_semantic() const;
	virtual void					set_semantic( boost::shared_ptr<sasl::semantic::module_semantic> const& );
	virtual module_context*			get_context() const;
	virtual void					set_context( boost::shared_ptr<module_context> const& );

	virtual llvm::Module*			llvm_module() const;
	virtual llvm::LLVMContext&		llvm_context();
	virtual llvm::Module*			take_ownership() const;
	virtual llvm::DefaultIRBuilder*	builder() const;

	virtual void dump_ir() const;
	virtual void dump_ir( std::ostream& ostr ) const;

	
	llvm::Type*	entry_param_type( salviar::sv_usage st ) const;
	void		entry_param_type( salviar::sv_usage st, llvm::Type* t );

	~cg_module_impl();

protected:
	boost::shared_ptr<llvm::LLVMContext>		llvm_ctxt_;
	boost::shared_ptr<llvm::DefaultIRBuilder>	irbuilder_;
	boost::shared_ptr<sasl::semantic::module_semantic>
												sem_;
	boost::shared_ptr<sasl::codegen::module_context>
												ctxt_;
	llvm::Module*								llvm_mod_;
	mutable bool								have_mod_;

	llvm::Type* param_types[salviar::storage_usage_count];
};

END_NS_SASL_CODEGEN();

#endif
