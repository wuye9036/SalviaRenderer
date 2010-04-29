#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/code_generator/llvm/llvm_generator.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

boost::shared_ptr<llvm::Module> create_module( const std::string& name, llvm::LLVMContext& ctxt ){
	return boost::shared_ptr<llvm::Module>( new llvm::Module(name, ctxt) );
}

llvm::FunctionType* create_parameters( boost::shared_ptr< sasl::syntax_tree::parameter_list > params ){
	return NULL;
}

llvm::Type* return_type(  boost::shared_ptr< sasl::syntax_tree::parameter_list > params ){
	return llvm::IntegerType::get();
}

boost::shared_ptr<llvm::Function> create_function(
	boost::shared_ptr<llvm::Module> mod,
	boost::shared_ptr<sasl::syntax_tree::function_type> function_info
	){
	return (Function*)(mod->getOrInsertFunction( function_info->name->lit, create_parameters(function_info->params)) );
}

void name_test(){

}