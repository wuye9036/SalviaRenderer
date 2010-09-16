#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_GLOBALCTXT_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_GLOBALCTXT_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>

#include <eflib/include/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/enable_warnings.h>

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

	virtual boost::shared_ptr<llvm::Module> module() const;
	virtual llvm::LLVMContext& context();

	~cgllvm_global_context();
private:
	boost::shared_ptr<llvm::LLVMContext> lctxt;
	boost::shared_ptr<llvm::Module> mod;
	boost::shared_ptr<llvm::IRBuilder<> > irbuilder;
};

END_NS_SASL_CODE_GENERATOR();

#endif