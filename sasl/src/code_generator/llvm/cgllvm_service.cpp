#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Function.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/diagnostics/assert.h>

using sasl::syntax_tree::function_type;
using sasl::syntax_tree::parameter;
using sasl::syntax_tree::tynode;

using sasl::semantic::storage_si;

using namespace sasl::utility;

using llvm::Argument;
using llvm::LLVMContext;
using llvm::Function;
using llvm::FunctionType;
using llvm::IntegerType;
using llvm::Type;
using llvm::PointerType;
using llvm::Value;
using llvm::BasicBlock;
using llvm::Constant;
using llvm::StructType;
using llvm::VectorType;

using boost::shared_ptr;

using std::vector;
using std::string;

BEGIN_NS_SASL_CODE_GENERATOR();

// Value tyinfo
value_tyinfo::value_tyinfo(
	tynode* sty,
	llvm::Type const* cty,
	llvm::Type const* llty
	) : sty(sty)
{
	llvm_tys[abi_c] = cty;
	llvm_tys[abi_llvm] = llty;
}

value_tyinfo::value_tyinfo()
	:sty(NULL), cls( unknown_type )
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

tynode* value_tyinfo::get_typtr() const{
	return sty;
}

shared_ptr<tynode> value_tyinfo::get_tysp() const{
	return sty->as_handle<tynode>();
}

llvm::Type const* value_tyinfo::get_llvm_ty( abis abi ) const
{
	return llvm_tys[abi];
}

/// @}

/// value_t @{
value_t::value_t()
	: tyinfo(NULL), val(NULL), cg(NULL), kind(kind_unknown), hint(builtin_types::none)
{
}

value_t::value_t( value_tyinfo* tyinfo, llvm::Value* val, value_t::kinds k, cg_service* cg )
	: tyinfo(tyinfo), val(val), cg(cg), kind(k), hint(builtin_types::none)
{
}

value_t::value_t( builtin_types hint, llvm::Value* val, value_t::kinds k, cg_service* cg )
	: tyinfo(NULL), hint(builtin_types::none)
{

}

abis value_t::get_abi() const{
	return abi;
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
	return val;
}

value_t value_t::to_rvalue() const
{
	return value_t( tyinfo, load_llvm_value(), kind_value, cg );
}

builtin_types value_t::get_hint() const
{
	return tyinfo->get_hint();
}

llvm::Value* value_t::load_llvm_value() const{
	switch( kind ){
	case kind_value:
		return val;
	case kind_ref:
		return cg->builder()->CreateLoad( val );
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return NULL;
	}
}

value_t::kinds value_t::get_kind() const{
	return kind;
}

bool value_t::is_lvalue() const{
	switch( kind ){
	case kind_ref:
	case kind_global:
	case kind_local:
		return true;
	case kind_value:
	case kind_unknown:
		return false;
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return false;
	}
}

bool value_t::storable() const{
	return is_lvalue() || ( val == NULL && kind == kind_unknown );
}

void value_t::set_value( Value* v, kinds k ){
	val = v;
	kind = k;
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
	// TODO: assert( *lhs.get_tyinfo() == *rhs.get_tyinfo() );
	assert( lhs.storable() );
	value_t rv = rhs.to_rvalue();
	switch( lhs.get_kind() ){
	case value_t::kind_local:
	case value_t::kind_ref:
	case value_t::kind_global:
		builder()->CreateStore( rv.load_llvm_value(), lhs.get_llvm_value() );
		break;
	case value_t::kind_unknown:
		// Copy directly.
		lhs.set_value( rhs.get_llvm_value(), rhs.kind );
		lhs.tyinfo = rhs.tyinfo;
		break;
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
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

value_t cg_service::create_vector( std::vector<value_t> const& scalars, abis abi ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

void cg_service::emit_return(){
	builder()->CreateRetVoid();
}

void cg_service::emit_return( value_t const& ret_v ){
	builder()->CreateRet( ret_v.to_rvalue().load_llvm_value() );
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

	if( is_vector(bt) ){
		Type const* elem_ty = create_llvm_type( ctxt, scalar_of(bt), is_c_compatible );
		size_t vec_size = vector_size(bt);
		if( is_c_compatible ){
			vector<Type const*> elem_tys(vec_size, elem_ty);
			return StructType::get( ctxt, elem_tys );
		} else {
			return VectorType::get( elem_ty, static_cast<unsigned int>(vec_size) );
		}
	}

	if( is_matrix(bt) ){
		Type const* vec_ty = create_llvm_type( ctxt, vector_of( scalar_of(bt), vector_size(bt) ), is_c_compatible );
		vector<Type const*> row_tys( vector_count(bt), vec_ty );
		return StructType::get( ctxt, row_tys );
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

shared_ptr<value_tyinfo> cg_service::create_tyinfo( shared_ptr<tynode> const& tyn ){
	value_tyinfo* ret = new value_tyinfo();
	ret->sty = tyn.get();
	ret->cls = value_tyinfo::unknown_type;

	if( tyn->is_builtin() ){
		ret->llvm_tys[abi_c] = create_llvm_type( context(), tyn->tycode, true );
		ret->llvm_tys[abi_llvm] = create_llvm_type( context(), tyn->tycode, false );
		ret->cls = value_tyinfo::builtin;
	} else {
		ret->cls = value_tyinfo::aggregated;
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return shared_ptr<value_tyinfo>(ret);
}

function_t cg_service::create_function( shared_ptr<function_type> const& fn_node ){
	function_t ret;
	ret.fnty = fn_node.get();
	ret.c_compatible = fn_node->si_ptr<storage_si>()->c_compatible();

	if( ret.c_compatible ){
		EFLIB_ASSERT_UNIMPLEMENTED();
		return function_t();
	}

	abis abi = ret.c_compatible ? abi_c : abi_llvm;

	vector<Type const*> par_tys;

	BOOST_FOREACH( shared_ptr<parameter> const& par, fn_node->params )
	{
		cgllvm_sctxt* par_ctxt = node_ctxt( par, false );
		value_tyinfo* par_ty = par_ctxt->get_typtr();
		assert( par_ty );

		bool is_ref = par->si_ptr<storage_si>()->is_reference();

		Type const* par_llty = par_ty->get_llvm_ty( abi ); 
		if( ret.c_compatible && is_ref ){
			par_tys.push_back( PointerType::getUnqual( par_llty ) );
		} else {
			par_tys.push_back( par_llty );
		}
	}

	Type const* rety = node_ctxt( fn_node->retval_type, false )->get_typtr()->get_llvm_ty( abi );
	FunctionType* fty = FunctionType::get( rety, par_tys, false );

	ret.fn = Function::Create( fty, Function::ExternalLinkage, fn_node->symbol()->mangled_name(), module() );
	ret.cg = this;

	return ret;
}

bool cg_service::in_function() const{
	return !fn_ctxts.empty();
}

void cg_service::clean_empty_blocks()
{
	assert( in_function() );

	typedef Function::BasicBlockListType::iterator block_iterator_t;
	block_iterator_t beg = fn().fn->getBasicBlockList().begin();
	block_iterator_t end = fn().fn->getBasicBlockList().end();

	for(  block_iterator_t it = beg; it != end; ++it )
	{
		if( !it->getTerminator() ){
			block_iterator_t next_it = it;
			++next_it;

			builder()->SetInsertPoint( &(*it) );

			if( next_it != fn().fn->getBasicBlockList().end() ){
				builder()->CreateBr( &(*next_it) );
			} else {
				if( !fn().fn->getReturnType()->isVoidTy() ){
					builder()->CreateRet( Constant::getNullValue( fn().fn->getReturnType() ) );
				} else {
					emit_return();
				}
			}
		}
	}
}

value_t cg_service::create_scalar( Value* val, value_tyinfo* tyinfo ){
	return value_t( tyinfo, val, value_t::kind_value, this );
}

BasicBlock* cg_service::new_block( std::string const& hint, bool set_insert_point )
{
	assert( in_function() );

	BasicBlock* ret = BasicBlock::Create( context(), hint, fn().fn );
	if( set_insert_point ){
		builder()->SetInsertPoint(ret);
	}
	return ret;
}

value_t cg_service::create_value( value_tyinfo* tyinfo, Value* val, value_t::kinds k ){
	return value_t( tyinfo, val, k, this );
}

sasl::code_generator::value_t cg_service::emit_mul( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

sasl::code_generator::value_t cg_service::emit_add( value_t const& lhs, value_t const& rhs )
{
	builtin_types hint = lhs.get_hint();

	assert( hint != builtin_types::none );
	assert( is_scalar( scalar_of( hint ) ) );
	assert( hint == rhs.get_hint() );

	if( is_scalar(hint) ){
		return emit_add_ss(lhs, rhs);
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::emit_add_ss( value_t const& lhs, value_t const& rhs )
{
	builtin_types hint = lhs.get_hint();

	Value* lval = lhs.load_llvm_value();
	Value* rval = rhs.load_llvm_value();

	Value* ret = NULL;
	if( is_integer(hint) ){
		Value* ret = builder()->CreateAdd(lval, rval);
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return value_t( hint, ret, value_t::kind_value, this );
}

value_t operator+( value_t const& lhs, value_t const& rhs ){
	return lhs.cg->emit_add(lhs, rhs);
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
	assert( fnty->is_function() );
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<value_tyinfo>();
}

size_t function_t::arg_size() const{
	assert( fn );
	return fn ? fn->arg_size() : 0;
}

value_t function_t::arg( size_t index ) const
{
	shared_ptr<parameter> par = fnty->params[index];
	value_tyinfo* par_typtr = cg->node_ctxt( par, false )->get_typtr();

	if( argCache.empty() ){
		for( Function::ArgumentListType::iterator it = fn->arg_begin(); it != fn->arg_end(); ++it ){
			// Non const.
			const_cast<function_t*>(this)->argCache.push_back((Argument*)it);
		}
	}

	return cg->create_value( par_typtr, argCache[index], arg_is_ref(index) ? value_t::kind_ref : value_t::kind_value );
}

function_t::function_t(): fn(NULL), fnty(NULL)
{
}

bool function_t::arg_is_ref( size_t index ) const{
	assert( index < fnty->params.size() );
	return fnty->params[index]->si_ptr<storage_si>()->is_reference();
}

END_NS_SASL_CODE_GENERATOR();