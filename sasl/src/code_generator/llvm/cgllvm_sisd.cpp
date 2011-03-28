#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>
#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_helper.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Constants.h>
#include <eflib/include/platform/enable_warnings.h>

using namespace llvm;
using namespace sasl::syntax_tree;

BEGIN_NS_SASL_CODE_GENERATOR();

Constant* cgllvm_sisd::zero_value( boost::shared_ptr<type_specifier> typespec )
{
	if( typespec->node_class() == syntax_node_types::builtin_type ){
		builtin_type_code btc = typespec->value_typecode;
		if( sasl_ehelper::is_integer( btc ) ){
			return ConstantInt::get( node_ctxt(typespec)->type, 0, sasl_ehelper::is_signed(btc) );
		}
		if( sasl_ehelper::is_real( btc ) ){
			return ConstantFP::get( node_ctxt(typespec)->type, 0.0 );
		}
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

cgllvm_sctxt* cgllvm_sisd::node_ctxt( sasl::syntax_tree::node& v, bool create_if_need /*= false */ )
{
	return cgllvm_impl::node_ctxt<cgllvm_sctxt>(v, create_if_need);
}

END_NS_SASL_CODE_GENERATOR();