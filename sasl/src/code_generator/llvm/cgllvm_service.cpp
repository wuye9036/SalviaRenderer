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

using llvm::APInt;
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
using llvm::ConstantInt;
using llvm::ConstantVector;
using llvm::StructType;
using llvm::VectorType;
using llvm::UndefValue;

using boost::shared_ptr;
using boost::enable_if;
using boost::is_integral;

using std::vector;
using std::string;

// Fn name is function name, op_name is llvm Create##op_name/CreateF##op_name
#define EMIT_OP_SS_BODY( op_name )	\
		assert( lhs.get_hint() == rhs.get_hint() ); \
		assert( is_scalar(lhs.get_hint()) ); \
		\
		builtin_types hint( lhs.get_hint() ); \
		Value* ret = NULL; \
		\
		if( is_real(hint) ){ \
			ret = builder()->CreateF##op_name ( lhs.load(abi_llvm), rhs.load(abi_llvm) ); \
		} else { \
			ret = builder()->Create##op_name( lhs.load(abi_llvm), rhs.load(abi_llvm) ); \
		}	\
		\
		return value_t( hint, ret, value_t::kind_value, abi_llvm, this );

BEGIN_NS_SASL_CODE_GENERATOR();

namespace {
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

	template <typename T>
	APInt apint( T v ){
		return APInt( sizeof(v) << 3, static_cast<uint64_t>(v), boost::is_signed<T>::value );
	}

	void mask_to_indexes( char indexes[4], uint32_t mask ){
		for( int i = 0; i < 4; ++i ){
			// XYZW is 1,2,3,4 but LLVM used 0,1,2,3
			char comp_index = static_cast<char>( (mask >> i*8) & 0xFF );
			if( comp_index == 0 ){
				indexes[i] = -1;
				break;
			}
			indexes[i] = comp_index - 1;
		}
	}

	void indexes_to_mask( char indexes[4], uint32_t& mask ){
		mask = 0;
		for( int i = 0; i < 4; ++i ){
			mask += (uint32_t)( (indexes[i] + 1) << (i*8) );
		}
	}

	void print_blocks( Function* fn ){
		printf( "Function: 0x%X\n", fn );
		for( Function::BasicBlockListType::iterator it = fn->getBasicBlockList().begin(); it != fn->getBasicBlockList().end(); ++it ){
			printf( "  Block: 0x%X\n", &(*it) );
		}
	}
}

template <typename T>
ConstantInt* cg_service::int_( T v )
{
	return ConstantInt::get( context(), apint(v) );
}

template <typename T>
llvm::ConstantVector* cg_service::vector_( T const* vals, size_t length, typename enable_if< is_integral<T> >::type* )
{
	assert( vals && length > 0 );
	
	vector<Constant*> elems(length);
	for( size_t i = 0; i < length; ++i ){
		elems.push_back( int_(vals[i]) );
	}

	VectorType* vec_type = VectorType::get( elems[0]->getType(), length );
	return llvm::cast<ConstantVector>( ConstantVector::get( vec_type, elems ) );
}

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

builtin_types value_tyinfo::hint() const{
	if( !sty || !sty->is_builtin() ){
		return builtin_types::none;
	}
	return sty->tycode;
}

tynode* value_tyinfo::typtr() const{
	return sty;
}

shared_ptr<tynode> value_tyinfo::tysp() const{
	return sty->as_handle<tynode>();
}

llvm::Type const* value_tyinfo::llvm_ty( abis abi ) const
{
	return llvm_tys[abi];
}

/// @}

/// value_t @{
value_t::value_t()
	: tyinfo(NULL), val(NULL), cg(NULL), kind(kind_unknown), hint(builtin_types::none), abi(abi_unknown), masks(0)
{
}

value_t::value_t(
	value_tyinfo* tyinfo,
	llvm::Value* val, value_t::kinds k, abis abi,
	cg_service* cg 
	) 
	: tyinfo(tyinfo), val(val), cg(cg), kind(k), hint(builtin_types::none), abi(abi), masks(0)
{
}

value_t::value_t( builtin_types hint,
	llvm::Value* val, value_t::kinds k, abis abi,
	cg_service* cg 
	)
	: tyinfo(NULL), hint(hint), abi(abi), val(val), kind(k), cg(cg), masks(0)
{

}

value_t::value_t( value_t const& rhs )
	: tyinfo(rhs.tyinfo), hint(rhs.hint), abi(rhs.abi), val( rhs.val ), kind(rhs.kind), cg(rhs.cg), masks(rhs.masks)
{
	set_parent(rhs.parent.get());
}

abis value_t::get_abi() const{
	return abi;
}

value_t value_t::swizzle( size_t swz_code ) const{
	assert( is_vector( get_hint() ) );
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

llvm::Value* value_t::raw() const{
	return val;
}

value_t value_t::to_rvalue() const
{
	return value_t( tyinfo, load( abi ), kind_value, abi, cg );
}

builtin_types value_t::get_hint() const
{
	if( tyinfo ) return tyinfo->hint();
	return hint;
}

llvm::Value* value_t::load( abis abi ) const{
	assert( abi != abi_unknown );

	if( abi != get_abi() ){
		if( is_scalar( get_hint() ) ){
			return load();
		} else if( is_vector( get_hint() ) ){
			Value* org_value = load();
			
			value_t ret_value = cg->null_value( get_hint(), abi );
				
			size_t vec_size = vector_size( get_hint() );
			for( size_t i = 0; i < vec_size; ++i ){
				ret_value = cg->emit_insert_val( ret_value, (int)i, cg->emit_extract_elem(*this, i) );
			}

			return ret_value.load();
		} else if( is_matrix( get_hint() ) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
		return NULL;
	} else {
		return load();
	}
}

llvm::Value* value_t::load() const{
	switch( kind ){
	case kind_value:
		return val;
	case kind_ref:
		return cg->builder()->CreateLoad( val );
	case kind_swizzle:
		{
			// Decompose indexes.
			char indexes[4] = {-1, -1, -1, -1};
			mask_to_indexes(indexes, masks);
			vector<int> index_values;
			index_values.reserve(4);
			for( int i = 0; i < 4 && indexes[i] != -1; ++i ){
				index_values.push_back( indexes[i] );
			}
			assert( !index_values.empty() );

			// Swizzle
			Value* agg_v = parent->load();
			Value* ret = NULL;

			if( index_values.size() == 1 ){
				return cg->emit_extract_val( parent->to_rvalue(), index_values[0] ).load();
			} else {
				if( abi == abi_c ){
					EFLIB_ASSERT_UNIMPLEMENTED();
					return NULL;
				} else {
					Value* mask = cg->vector_( &indexes[0], index_values.size() );
					return cg->builder()->CreateShuffleVector( agg_v, UndefValue::get( agg_v->getType() ), mask );
				}
			}

			return ret;
		}
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return NULL;
	}
}

value_t::kinds value_t::get_kind() const{
	return kind;
}

bool value_t::storable() const{
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

bool value_t::load_only() const
{
	switch( kind ){
	case kind_ref:
	case kind_global:
	case kind_local:
	case kind_unknown:
		return false;
	case kind_value:
		return true;
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return false;
	}
}

void value_t::emplace( Value* v, kinds k, abis abi ){
	val = v;
	kind = k;
	this->abi = abi;
}

void value_t::emplace( value_t const& v )
{
	emplace( v.raw(), v.get_kind(), v.get_abi() );
}

llvm::Value* value_t::load_ref() const
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

value_tyinfo* value_t::get_tyinfo() const{
	return tyinfo;
}

value_t& value_t::operator=( value_t const& rhs )
{
	kind = rhs.kind;
	val = rhs.val;
	tyinfo = rhs.tyinfo;
	hint = rhs.hint;
	abi = rhs.abi;
	cg = rhs.cg;
	masks = rhs.masks;

	set_parent(rhs.parent.get());

	return *this;
}

void value_t::set_parent( value_t const& v )
{
	parent.reset( new value_t(v) );
}

void value_t::set_parent( value_t const* v )
{
	if(v){
		set_parent(*v);
	}
}

value_t value_t::slice( value_t const& vec, uint32_t masks )
{
	builtin_types hint = vec.get_hint();
	assert( is_vector(hint) );

	value_t ret( scalar_of(hint), NULL, kind_swizzle, vec.abi, vec.cg );
	ret.masks = masks;
	ret.set_parent(vec);

	return ret;
}

/// @}

void cg_service::store( value_t& lhs, value_t const& rhs ){
	// TODO: assert( *lhs.get_tyinfo() == *rhs.get_tyinfo() );
	assert( lhs.storable() );
	value_t rv = rhs.to_rvalue();
	switch( lhs.get_kind() ){
	case value_t::kind_local:
	case value_t::kind_ref:
	case value_t::kind_global:
		builder()->CreateStore( rv.load( lhs.get_abi() ), lhs.raw() );
		break;
	case value_t::kind_unknown:
		// Copy directly.
		lhs.emplace( rhs );
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

value_t cg_service::null_value( value_tyinfo* tyinfo, abis abi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::null_value( builtin_types bt, abis abi )
{
	assert( bt != builtin_types::none );
	Type const* valty = create_llvm_type( context(), bt, abi == abi_c );
	value_t val = value_t( bt, Constant::getNullValue( valty ), value_t::kind_value, abi, this );
	return val;
}

value_t cg_service::create_vector( std::vector<value_t> const& scalars, abis abi ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

void cg_service::emit_return(){
	builder()->CreateRetVoid();
}

void cg_service::emit_return( value_t const& ret_v ){
	builder()->CreateRet( ret_v.to_rvalue().load() );
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

function_t cg_service::fetch_function( shared_ptr<function_type> const& fn_node ){
	
	cgllvm_sctxt* fn_ctxt = node_ctxt( fn_node, false );
	if( fn_ctxt->data().self_fn ){
		return fn_ctxt->data().self_fn;
	}

	function_t ret;
	ret.fnty = fn_node.get();
	ret.c_compatible = fn_node->si_ptr<storage_si>()->c_compatible();

	abis abi = ret.c_compatible ? abi_c : abi_llvm;

	vector<Type const*> par_tys;

	// Create function type.
	BOOST_FOREACH( shared_ptr<parameter> const& par, fn_node->params )
	{
		cgllvm_sctxt* par_ctxt = node_ctxt( par, false );
		value_tyinfo* par_ty = par_ctxt->get_typtr();
		assert( par_ty );

//		bool is_ref = par->si_ptr<storage_si>()->is_reference();

		Type const* par_llty = par_ty->llvm_ty( abi ); 
		if( ret.c_compatible && !is_scalar(par_ty->hint()) ){
			par_tys.push_back( PointerType::getUnqual( par_llty ) );
		} else {
			par_tys.push_back( par_llty );
		}
	}

	Type const* ret_ty = node_ctxt( fn_node->retval_type, false )->get_typtr()->llvm_ty( abi );
	FunctionType* fty = FunctionType::get( ret_ty, par_tys, false );

	// Create function
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

	print_blocks( fn().fn );

	for( block_iterator_t it = beg; it != end; ++it )
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
	return value_t( tyinfo, val, value_t::kind_value, abi_llvm, this );
}

insert_point_t cg_service::new_block( std::string const& hint, bool set_as_current )
{
	assert( in_function() );

	insert_point_t ret;
	
	if( fn().fn->getBasicBlockList().empty() || !fn().fn->getBasicBlockList().back().empty() ){
		ret.block = BasicBlock::Create( context(), hint, fn().fn );
		printf("Block created: 0x%X at 0x%X\n", (uintptr_t)ret.block, fn().fn );

		print_blocks( fn().fn );
	} else {
		ret.block = &fn().fn->getBasicBlockList().back();
	}
	
	if( set_as_current ){
		set_insert_point( ret );
	}

	return ret;
}

value_t cg_service::create_value( value_tyinfo* tyinfo, Value* val, value_t::kinds k, abis abi ){
	return value_t( tyinfo, val, k, abi, this );
}

value_t cg_service::create_value( builtin_types hint, Value* val, value_t::kinds k, abis abi )
{
	return value_t( hint, val, k, abi, this );
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
	EMIT_OP_SS_BODY(Add);
}

void cg_service::set_insert_point( insert_point_t const& ip ){
	builder()->SetInsertPoint(ip.block);
}

value_t cg_service::emit_dot( value_t const& lhs, value_t const& rhs )
{
	return emit_dot_vv(lhs, rhs);
}

value_t cg_service::emit_dot_vv( value_t const& lhs, value_t const& rhs )
{
	size_t vec_size = vector_size( lhs.get_hint() );

	value_t total = null_value( scalar_of( lhs.get_hint() ), abi_llvm );

	for( size_t i = 0; i < vec_size; ++i ){
		value_t lhs_elem = emit_extract_elem( lhs, i );
		value_t rhs_elem = emit_extract_elem( rhs, i );

		value_t elem_mul = emit_mul_ss( lhs_elem, rhs_elem );
		total.emplace( emit_add_ss( total, elem_mul ).to_rvalue() );
	}

	return total;
}

value_t cg_service::emit_extract_ref( value_t const& lhs, int idx )
{
	assert( lhs.storable() );

	builtin_types agg_hint = lhs.get_hint();

	if( is_vector(agg_hint) ){
		char indexes[4] = { (char)idx, -1, -1, -1 };
		uint32_t mask = 0;
		indexes_to_mask( indexes, mask );
		return value_t::slice( lhs, mask );
	} else if( is_matrix(agg_hint) ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	} else if ( agg_hint == builtin_types::none ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
	
	return value_t();
}

value_t cg_service::emit_extract_ref( value_t const& lhs, value_t const& idx )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::emit_extract_val( value_t const& lhs, int idx )
{
	builtin_types agg_hint = lhs.get_hint();

	Value* val = lhs.load();
	Value* elem_val = NULL;
	abis abi = abi_unknown;
	builtin_types elem_hint = builtin_types::none;

	if( agg_hint == builtin_types::none ){
		EFLIB_ASSERT_UNIMPLEMENTED();
		return value_t();
	} else if( is_scalar(agg_hint) ){
		assert( idx == 0 );
		elem_val = val;
	} else if( is_vector(agg_hint) ){
		switch( lhs.get_abi() ){
		case abi_c:
			elem_val = builder()->CreateExtractValue(val, static_cast<unsigned>(idx));
			break;
		case abi_llvm:
			elem_val = builder()->CreateExtractElement(val, int_(idx) );
			break;
		default:
			assert(!"Unknown ABI");
			break;
		}
		abi = abi_llvm;
		elem_hint = scalar_of(agg_hint);
	} else if( is_matrix(agg_hint) ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return value_t(elem_hint, elem_val, value_t::kind_value, abi, this );
}

value_t cg_service::emit_extract_val( value_t const& lhs, value_t const& idx )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cg_service::emit_mul_ss( value_t const& lhs, value_t const& rhs )
{
	EMIT_OP_SS_BODY(Mul);
}

value_t cg_service::emit_call( function_t const& fn, vector<value_t> const& args )
{
	vector<Value*> arg_values;
	
	if( fn.c_compatible ){
		BOOST_FOREACH( value_t const& arg, args ){
			builtin_types hint = arg.get_hint();
			if( is_scalar(hint) ){
				arg_values.push_back( arg.load(abi_llvm) );
			} else {
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
			// arg_values.push_back( arg.load( abi_llvm ) );
		}
	} else {
		BOOST_FOREACH( value_t const& arg, args ){
			arg_values.push_back( arg.load( abi_llvm ) );
		}
	}

	Value* ret_val = builder()->CreateCall( fn.fn, arg_values.begin(), arg_values.end() );

	abis ret_abi = fn.c_compatible ? abi_c : abi_llvm;
	return value_t( fn.get_return_ty().get(), ret_val, value_t::kind_value, ret_abi, this );
}

value_t cg_service::emit_insert_val( value_t const& lhs, value_t const& idx, value_t const& elem_value )
{
	Value* indexes[1] = { idx.load() };
	Value* agg = lhs.load();
	Value* new_value = NULL;
	if( agg->getType()->isStructTy() ){
		assert(false);
	} else if ( agg->getType()->isVectorTy() ){
		new_value = builder()->CreateInsertElement( agg, elem_value.load(), indexes[0] );
	}
	assert(new_value);

	if( lhs.get_tyinfo() ){
		return value_t( lhs.get_tyinfo(), new_value, value_t::kind_value, lhs.get_abi(), this );
	} else {
		return value_t( lhs.get_hint(), new_value, value_t::kind_value, lhs.get_abi(), this );
	}
}

value_t cg_service::emit_insert_val( value_t const& lhs, int index, value_t const& elem_value )
{
	Value* agg = lhs.load();
	Value* new_value = NULL;
	if( agg->getType()->isStructTy() ){
		new_value = builder()->CreateInsertValue( agg, elem_value.load(), (unsigned)index );
	} else if ( agg->getType()->isVectorTy() ){
		value_t index_value( builtin_types::_sint32, int_(index), value_t::kind_value, abi_llvm, this );
		return emit_insert_val( lhs, index_value, elem_value );
	}
	assert(new_value);

	if( lhs.get_tyinfo() ){
		return value_t( lhs.get_tyinfo(), new_value, value_t::kind_value, lhs.get_abi(), this );
	} else {
		return value_t( lhs.get_hint(), new_value, value_t::kind_value, lhs.get_abi(), this );
	}

	
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

shared_ptr<value_tyinfo> function_t::get_return_ty() const{
	assert( fnty->is_function() );
	return shared_ptr<value_tyinfo>( cg->node_ctxt( fnty->retval_type, false )->get_tysp() );
}

size_t function_t::arg_size() const{
	assert( fn );
	return fn ? fn->arg_size() : 0;
}

value_t function_t::arg( size_t index ) const
{
	shared_ptr<parameter> par = fnty->params[index];
	value_tyinfo* par_typtr = cg->node_ctxt( par, false )->get_typtr();

	Function::ArgumentListType::iterator it = fn->arg_begin();
	for( size_t idx_counter = 0; idx_counter < index; ++idx_counter ){
		++it;
	}

	abis arg_abi = c_compatible ? abi_c: abi_llvm;
	return cg->create_value( par_typtr, &(*it), arg_is_ref(index) ? value_t::kind_ref : value_t::kind_value, arg_abi );
}

function_t::function_t(): fn(NULL), fnty(NULL)
{
}

bool function_t::arg_is_ref( size_t index ) const{
	assert( index < fnty->params.size() );
	builtin_types hint = fnty->params[index]->si_ptr<storage_si>()->type_info()->tycode;
	return c_compatible && !is_scalar(hint);
}


insert_point_t::insert_point_t(): block(NULL)
{
}

END_NS_SASL_CODE_GENERATOR();