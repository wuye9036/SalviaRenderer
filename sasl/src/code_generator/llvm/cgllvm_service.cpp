#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Function.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/diagnostics/assert.h>

using sasl::syntax_tree::tynode;

using namespace sasl::utility;

using llvm::LLVMContext;
using llvm::Function;
using llvm::IntegerType;
using llvm::Type;

using boost::shared_ptr;

using std::vector;
using std::string;

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

value_tyinfo::value_tyinfo()
	:sty(NULL), abi(abi_unknown), ty( unknown_type ), hint( builtin_types::none )
{
	llvm_tys[0] = NULL;
	llvm_tys[1] = NULL;
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

tynode* value_tyinfo::get_typtr() const{
	return sty;
}

shared_ptr<tynode> value_tyinfo::get_tysp() const{
	return sty->as_handle<tynode>();
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

void cg_service::emit_return(){
	builder()->CreateRetVoid();
}

void cg_service::emit_return( value_t const& ret_v ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

function_t& cg_service::fn(){
	return fn_ctxts.back();
}

void cg_service::push_fn( function_t const& fn ){
	fn_ctxts.push_back(fn);
}

void cg_service::pop_fn(){
	fn_ctxts.pop_back();
}

/// Create LLVM type from builtin types.
Type const* create_llvm_type( LLVMContext& ctxt, builtin_types bt, bool is_c_compatible ){
	assert( bt != builtin_types::none );

	if ( is_void( bt ) ){
		return Type::getVoidTy( ctxt );
	}

	if( is_scalar(bt) ){
		if( bt == builtin_types::_boolean ){
			return IntegerType::get( ctxt, 1 );
		}
		if( is_integer(bt) ){
			return IntegerType::get( ctxt, (unsigned int)storage_size( bt ) << 3 );
		}
		if ( bt == builtin_types::_float ){
			return Type::getFloatTy( ctxt );
		}
		if ( bt == builtin_types::_double ){
			return Type::getDoubleTy( ctxt );
		}
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

shared_ptr<value_tyinfo> cg_service::create_tyinfo( shared_ptr<tynode> const& tyn ){
	value_tyinfo* ret = new value_tyinfo();
	ret->sty = tyn.get();
	ret->hint = builtin_types::none;

	if( tyn->is_builtin() ){
		ret->llvm_tys[value_tyinfo::abi_c] = create_llvm_type( context(), tyn->tycode, true );
		ret->llvm_tys[value_tyinfo::abi_llvm] = create_llvm_type( context(), tyn->tycode, false );
		ret->hint = tyn->tycode;
		ret->ty = value_tyinfo::builtin;
	} else {
		ret->ty = value_tyinfo::aggregated;
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return shared_ptr<value_tyinfo>(ret);
}

value_t operator+( value_t const& lhs, value_t const& rhs ){
	assert( lhs.get_hint() != builtin_types::none );
	assert( is_scalar( scalar_of( lhs.get_hint() ) ) );

	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}


void function_t::arg_name( size_t index, std::string const& name ){
	assert( index < fn->arg_size() );
	Function::arg_iterator arg_it = fn->arg_begin();
	for( size_t i = 0; i < index; ++i ){ ++arg_it; }
	arg_it->setName( name );
}

void function_t::args_name( vector<string> const& names )
{
	assert( names.size() <= fn->arg_size() );

	Function::arg_iterator arg_it = fn->arg_begin();
	vector<string>::const_iterator name_it = names.begin();

	for( size_t i = 0; i < names.size(); ++i ){
		arg_it->setName( *name_it );
		++arg_it;
		++name_it;
	}
}

shared_ptr<value_tyinfo> function_t::get_return_ty(){
	assert( fnty->get_typtr()->is_function() );
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<value_tyinfo>();
}

size_t function_t::arg_size() const{
	assert( fn );
	return fn ? fn->arg_size() : 0;
}

value_t function_t::arg( size_t index ) const
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

function_t::function_t(): fn(NULL), fnty(NULL)
{
}

END_NS_SASL_CODE_GENERATOR();
