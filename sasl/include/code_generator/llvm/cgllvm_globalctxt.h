#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_GLOBALCTXT_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_GLOBALCTXT_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>

namespace sasl{
	namespace semantic{
		enum storage_types;
	}
}

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

#include <boost/shared_ptr.hpp>
#include <string>

BEGIN_NS_SASL_CODE_GENERATOR();

/////////////////////////////////////////
// support LLVMContext, Module, IRBuilder and other data use by code generator.
//
class cgllvm_global_context: public llvm_code{
public:
	cgllvm_global_context();
	void create_module( const std::string& modname );

	virtual llvm::Module* module() const;
	virtual boost::shared_ptr<llvm::DefaultIRBuilder> builder() const;
	virtual llvm::Module* get_ownership() const;

	virtual llvm::LLVMContext& context();

	llvm::Type const* get_type( sasl::semantic::storage_types storage );
	void set_type( sasl::semantic::storage_types, llvm::Type const* );
	~cgllvm_global_context();
private:
	boost::shared_ptr<llvm::LLVMContext> lctxt;
	boost::shared_ptr<llvm::DefaultIRBuilder> irbuilder;
	
	llvm::Module* mod;

	llvm::Type const* str_in_struct;
	llvm::Type const* str_out_struct;
	llvm::Type const* buf_in_struct;
	llvm::Type const* buf_out_struct;

	mutable bool have_mod;
};

END_NS_SASL_CODE_GENERATOR();

#endif
