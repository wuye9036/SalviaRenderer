#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/builtin_type_code.h>
#include <sasl/enums/enums_helper.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/diagnostics/assert.h>

using namespace sasl::syntax_tree;
using namespace llvm;

BEGIN_NS_SASL_CODE_GENERATOR();

#define SASL_VISITOR_TYPE_NAME cgllvm_impl

cgllvm_impl::cgllvm_impl(): abii(NULL), msi(NULL){
}

SASL_VISIT_DEF_UNIMPL( declaration );

Type const* cgllvm_impl::llvm_type( builtin_type_code const& btc, bool& sign ){

	if ( sasl_ehelper::is_void( btc ) ){
		return Type::getVoidTy( mod->context() );
	}
	
	if( sasl_ehelper::is_scalar(btc) ){
		if( btc == builtin_type_code::_boolean ){
			return IntegerType::get( mod->context(), 1 );
		}
		if( sasl_ehelper::is_integer(btc) ){
			sign = sasl_ehelper::is_signed( btc );
			return IntegerType::get( mod->context(), (unsigned int)sasl_ehelper::storage_size( btc ) << 3 );
		}
		if ( btc == builtin_type_code::_float ){
			return Type::getFloatTy( mod->context() );
		}
		if ( btc == builtin_type_code::_double ){
			return Type::getDoubleTy( mod->context() );
		}
	} 
	
	if( sasl_ehelper::is_vector( btc) ){
		builtin_type_code scalar_btc = sasl_ehelper::scalar_of( btc );
		Type const* inner_type = llvm_type(scalar_btc, sign);
		return VectorType::get( inner_type, static_cast<uint32_t>(sasl_ehelper::len_0(btc)) );
	}
	
	if( sasl_ehelper::is_matrix( btc ) ){
		builtin_type_code scalar_btc = sasl_ehelper::scalar_of( btc );
		Type const* row_type =
			llvm_type( sasl_ehelper::vector_of(scalar_btc, sasl_ehelper::len_0(btc)), sign );
		return ArrayType::get( row_type, sasl_ehelper::len_1(btc) );
	}

	return NULL;
}

llvm::DefaultIRBuilder* cgllvm_impl::builder(){
	return mod->builder().get();
}

boost::shared_ptr<llvm_module> cgllvm_impl::module()
{
	return mod;
}

llvm::LLVMContext& cgllvm_impl::llcontext(){
	return mod->context();
}

llvm::Module* cgllvm_impl::llmodule() const{
	return mod->module();
}

END_NS_SASL_CODE_GENERATOR();