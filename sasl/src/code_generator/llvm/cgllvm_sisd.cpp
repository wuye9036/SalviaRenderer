#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>
#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_helper.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/BasicBlock.h>
#include <llvm/Constants.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

using boost::any_cast;

using namespace llvm;
using namespace sasl::syntax_tree;


BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_sctxt const * sc_ptr( const boost::any& any_val ){
	return any_cast<cgllvm_sctxt>(&any_val);
}

cgllvm_sctxt* sc_ptr( boost::any& any_val ){
	return any_cast<cgllvm_sctxt>(&any_val);
}

#define data_as_sc_ptr() ( sc_ptr(*data) )

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

void cgllvm_sisd::restart_block( boost::any* data ){
	BasicBlock* restart = BasicBlock::Create( llcontext(), "", data_as_sc_ptr()->parent_func );
	builder()->SetInsertPoint(restart);
}
END_NS_SASL_CODE_GENERATOR();