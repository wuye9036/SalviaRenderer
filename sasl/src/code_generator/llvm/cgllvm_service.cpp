#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_utility.h>

using sasl::syntax_tree::tynode;
using namespace sasl::utility;

BEGIN_NS_SASL_CODE_GENERATOR();

/// value_tyinfo @{
value_tyinfo::value_tyinfo(
	tynode* sty,
	llvm::Type const* cty,
	llvm::Type const* llty,
	value_tyinfo::abis abi
	) : sty(sty), abi(abi), hint(builtin_types::none)
{
	llvm_tys[abi_c] = cty;
	llvm_tys[abi_llvm] = llty;

	if( sty->is_builtin() ){
		hint = sty->tycode;
	}
}
/// @}

/// value_proxy @{
value_proxy::value_proxy()
	: tyinfo(NULL), val(NULL), cg(NULL)
{
}

value_proxy::value_proxy( value_tyinfo* tyinfo, llvm::Value* val, cg_service* cg )
	: tyinfo(tyinfo), val(val), cg(cg)
{
}

/// @}

/// cgv_scalar @{
cgv_scalar operator+( cgv_scalar const& lhs, cgv_scalar const& rhs ){
	cg_service* cgs = lhs.service();

	value_tyinfo* lhs_ti = lhs.get_tyinfo();
	value_tyinfo* rhs_ti = rhs.get_tyinfo();

	builtin_types value_hint = lhs_ti->get_hint();

	assert( is_scalar(value_hint) && value_hint == rhs_ti->get_hint() );
	
	llvm::Value* ret_llval = NULL;

	if( value_hint == builtin_types::_float
		|| value_hint == builtin_types::_double 
		)
	{
		ret_llval = cgs->builder()->CreateFAdd( lhs.get_value(), rhs.get_value(), "" );
	} else if( is_integer( value_hint ) ){
		ret_llval = cgs->builder()->CreateAdd( lhs.get_value(), rhs.get_value(), "" );
	}

	assert( ret_llval );

	return cgs->create_scalar( ret_llval, lhs_ti );
}

/// @}
END_NS_SASL_CODE_GENERATOR();
