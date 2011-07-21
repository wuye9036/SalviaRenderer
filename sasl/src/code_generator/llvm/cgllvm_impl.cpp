#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/diagnostics/assert.h>

#include <vector>

using namespace sasl::syntax_tree;
using namespace llvm;
using namespace sasl::utility;

using std::vector;

BEGIN_NS_SASL_CODE_GENERATOR();

#define SASL_VISITOR_TYPE_NAME cgllvm_impl

cgllvm_impl::cgllvm_impl(): abii(NULL), msi(NULL){
}

SASL_VISIT_DEF_UNIMPL( declaration );

Type const* cgllvm_impl::llvm_type( builtin_types const& btc, bool& as_vector, bool& is_mat ){
	as_vector = false;
	is_mat = false;

	if ( is_void( btc ) ){
		return Type::getVoidTy( mod->context() );
	}
	
	if( is_scalar(btc) ){
		if( btc == builtin_types::_boolean ){
			return IntegerType::get( mod->context(), 1 );
		}
		if( is_integer(btc) ){
			return IntegerType::get( mod->context(), (unsigned int)storage_size( btc ) << 3 );
		}
		if ( btc == builtin_types::_float ){
			return Type::getFloatTy( mod->context() );
		}
		if ( btc == builtin_types::_double ){
			return Type::getDoubleTy( mod->context() );
		}
	} 
	
	if( is_vector( btc) ){
		builtin_types scalar_btc = scalar_of( btc );
		Type const* inner_type = llvm_type(scalar_btc, as_vector, is_mat);
		as_vector = true;
		is_mat = false;
		vector<Type const*> member_tys;
		member_tys.insert( member_tys.end(), vector_size(btc), inner_type );
		return StructType::get( llcontext(), member_tys, true);
	}
	
	if( is_matrix( btc ) ){
		builtin_types scalar_btc = scalar_of( btc );
		Type const* row_type =
			llvm_type( vector_of(scalar_btc, vector_size(btc)), as_vector, is_mat );
		as_vector = true;
		is_mat = true;
		return ArrayType::get( row_type, vector_count(btc) );
	}

	return NULL;
}

void cgllvm_impl::llvm_type( builtin_types const& btc, cgllvm_sctxt* out_ctxt )
{
	out_ctxt->data().val_type
		= llvm_type( 
		btc,
		out_ctxt->data().as_vector,
		out_ctxt->data().is_matrix
		);
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