#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_GLOBALCTXT_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_GLOBALCTXT_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>

namespace llvm{
	class LLVMContext;
	class Module;
	class ConstantFolder;
	template <bool preserveNames> class IRBuilderDefaultInserter;
	template<
		bool preserveNames = true,
		typename T = ConstantFolder,
		typename Inserter = IRBuilderDefaultInserter<preserveNames> >
	class IRBuilder;
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
	virtual boost::shared_ptr<llvm::IRBuilder<> > builder() const;
	virtual llvm::Module* get_ownership() const;

	virtual llvm::LLVMContext& context();

	~cgllvm_global_context();
private:
	boost::shared_ptr<llvm::LLVMContext> lctxt;
	boost::shared_ptr<llvm::IRBuilder<> > irbuilder;
	llvm::Module* mod;
	mutable bool have_mod;
};

END_NS_SASL_CODE_GENERATOR();

#endif