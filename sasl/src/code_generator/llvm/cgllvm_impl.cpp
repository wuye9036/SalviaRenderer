#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/builtin_type_code.h>
#include <sasl/enums/enums_helper.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <eflib/include/platform/enable_warnings.h>

using namespace sasl::syntax_tree;
using namespace llvm;

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_impl::cgllvm_impl(){
}

sctxt_handle cgllvm_impl::node_ctxt( node& nd, bool create_if_need ){
	return node_ctxt(nd.handle(), create_if_need);
}

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
			llvm_type( sasl_ehelper::vector_of(scalar_btc, sasl_ehelper::len_1(btc)), sign );
		return ArrayType::get( row_type, sasl_ehelper::len_0(btc) );
	}

	return NULL;
}

END_NS_SASL_CODE_GENERATOR();