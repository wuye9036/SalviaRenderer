#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/diagnostics/assert.h>

using sasl::syntax_tree::tynode;
using namespace sasl::utility;

BEGIN_NS_SASL_CODE_GENERATOR();

// Value tyinfo
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

builtin_types value_tyinfo::get_hint() const{
	if( !sty || !sty->is_builtin() ){
		return builtin_types::none;
	}
	return sty->tycode;
}

value_tyinfo::abis value_tyinfo::get_abi() const{
	return abi;
}
/// @}

/// value_t @{
value_t::value_t()
	: tyinfo(NULL), val(NULL), cg(NULL)
{
}

value_t::value_t( value_tyinfo* tyinfo, llvm::Value* val, cg_service* cg )
	: tyinfo(tyinfo), val(val), cg(cg)
{
}

value_t value_t::swizzle( size_t swz_code ) const{
	assert( is_vector( get_hint() ) );
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

llvm::Value* value_t::get_llvm_value() const{
	if( get_hint() == builtin_types::none ){
		return NULL;
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

value_t value_t::to_rvalue() const
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

builtin_types value_t::get_hint() const
{
	return tyinfo->get_hint();
}
/// @}

/// cgv_scalar @{
//cgv_scalar operator+( cgv_scalar const& lhs, cgv_scalar const& rhs ){
//	cg_service* cgs = lhs.service();
//
//	value_tyinfo* lhs_ti = lhs.get_tyinfo();
//	value_tyinfo* rhs_ti = rhs.get_tyinfo();
//
//	builtin_types value_hint = lhs_ti->get_hint();
//
//	assert( is_scalar(value_hint) && value_hint == rhs_ti->get_hint() );
//	
//	llvm::Value* ret_llval = NULL;
//
//	if( value_hint == builtin_types::_float
//		|| value_hint == builtin_types::_double 
//		)
//	{
//		ret_llval = cgs->builder()->CreateFAdd( lhs.get_value(), rhs.get_value(), "" );
//	} else if( is_integer( value_hint ) ){
//		ret_llval = cgs->builder()->CreateAdd( lhs.get_value(), rhs.get_value(), "" );
//	}
//
//	assert( ret_llval );
//
//	return cgs->create_scalar( ret_llval, lhs_ti );
//}

/// @}

void cg_service::store( value_t& lhs, value_t const& rhs ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

value_t cg_service::cast_ints( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::cast_i2f( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::cast_f2i( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::cast_f2f( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::null_value( value_tyinfo* tyinfo )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::create_vector( std::vector<value_t> const& scalars, value_tyinfo::abis abi ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t operator+( value_t const& lhs, value_t const& rhs ){
	assert( lhs.get_hint() != builtin_types::none );
	assert( is_scalar( scalar_of( lhs.get_hint() ) ) );

	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

END_NS_SASL_CODE_GENERATOR();
