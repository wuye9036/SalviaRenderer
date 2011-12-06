#include <sasl/include/code_generator/llvm/cgs_sisd.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Intrinsics.h>
#include <llvm/Support/TypeBuilder.h>
#include <llvm/Support/CFG.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/platform/cpuinfo.h>

using sasl::syntax_tree::node;
using sasl::syntax_tree::function_type;
using sasl::syntax_tree::parameter;
using sasl::syntax_tree::tynode;
using sasl::syntax_tree::declaration;
using sasl::syntax_tree::variable_declaration;
using sasl::syntax_tree::struct_type;

using sasl::semantic::storage_si;
using sasl::semantic::type_info_si;

using eflib::support_feature;
using eflib::cpu_sse2;

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
using llvm::StoreInst;
using llvm::TypeBuilder;
using llvm::AttrListPtr;
using llvm::SwitchInst;

namespace Intrinsic = llvm::Intrinsic;

using boost::any;
using boost::shared_ptr;
using boost::enable_if;
using boost::is_integral;
using boost::unordered_map;
using boost::lexical_cast;

using std::vector;
using std::string;

// Fn name is function name, op_name is llvm Create##op_name/CreateF##op_name
#define EMIT_OP_SS_VV_BODY( op_name )	\
	builtin_types hint( lhs.hint() ); \
	assert( hint == rhs.hint() ); \
	assert( is_scalar(hint) || is_vector(hint) ); \
	\
	Value* ret = NULL; \
	\
	builtin_types scalar_hint = is_scalar(hint) ? hint : scalar_of(hint); \
	if( is_real( scalar_hint ) ){ \
	ret = builder().CreateF##op_name ( lhs.load(abi_llvm), rhs.load(abi_llvm) ); \
	} else { \
	ret = builder().Create##op_name( lhs.load(abi_llvm), rhs.load(abi_llvm) ); \
	}	\
	\
	value_t retval = create_value( hint, ret, vkind_value, abi_llvm ); \
	abis ret_abi = is_scalar(hint) ? abi_llvm : lhs.abi();\
	return create_value( hint, retval.load(ret_abi), vkind_value, ret_abi );

#define EMIT_CMP_EQ_NE_BODY( op_name ) \
	builtin_types hint = lhs.hint(); \
	assert( hint == rhs.hint() ); \
	assert( is_scalar(hint) || (hint == builtin_types::_boolean) ); \
	\
	Value* ret = NULL; \
	if( is_integer(hint) || (hint == builtin_types::_boolean) ){ \
	ret = builder().CreateICmp##op_name( lhs.load(), rhs.load() ); \
	} else if( is_real(hint) ) { \
	ret = builder().CreateFCmpU##op_name( lhs.load(), rhs.load() ); \
	} \
	\
	return create_value( builtin_types::_boolean, ret, vkind_value, abi_llvm );

#define EMIT_CMP_BODY( op_name ) \
	builtin_types hint = lhs.hint(); \
	assert( hint == rhs.hint() ); \
	assert( is_scalar(hint) ); \
	\
	Value* ret = NULL; \
	if( is_integer(hint) ){ \
	if( is_signed(hint) ){ \
	ret = builder().CreateICmpS##op_name( lhs.load(), rhs.load() ); \
	} else {\
	ret = builder().CreateICmpU##op_name( lhs.load(), rhs.load() ); \
	}\
	\
	} else if( is_real(hint) ) { \
	ret = builder().CreateFCmpU##op_name( lhs.load(), rhs.load() ); \
	} \
	\
	return create_value( builtin_types::_boolean, ret, vkind_value, abi_llvm );

BEGIN_NS_SASL_CODE_GENERATOR();

namespace {


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

	uint32_t indexes_to_mask( char indexes[4] ){
		uint32_t mask = 0;
		for( int i = 0; i < 4; ++i ){
			mask += (uint32_t)( (indexes[i] + 1) << (i*8) );
		}
		return mask;
	}

	uint32_t indexes_to_mask( char idx0, char idx1, char idx2, char idx3 ){
		char indexes[4] = { idx0, idx1, idx2, idx3 };
		return indexes_to_mask( indexes );
	}

	void dbg_print_blocks( Function* fn ){
#ifdef _DEBUG
		/*printf( "Function: 0x%X\n", fn );
		for( Function::BasicBlockListType::iterator it = fn->getBasicBlockList().begin(); it != fn->getBasicBlockList().end(); ++it ){
		printf( "  Block: 0x%X\n", &(*it) );
		}*/
		fn = fn;
#else
		fn = fn;
#endif
	}
}

template <typename T>
ConstantInt* cgs_sisd::int_( T v )
{
	return ConstantInt::get( context(), apint(v) );
}

template <typename T>
llvm::ConstantVector* cgs_sisd::vector_( T const* vals, size_t length, typename enable_if< is_integral<T> >::type* )
{
	assert( vals && length > 0 );

	vector<Constant*> elems(length);
	for( size_t i = 0; i < length; ++i ){
		elems[i] = int_(vals[i]);
	}

	return llvm::cast<ConstantVector>( ConstantVector::get( elems ) );
}


template <typename FunctionT>
Function* cgs_sisd::intrin_( int id )
{
	return intrins.get(id, module(), TypeBuilder<FunctionT, false>::get( context() ) );
}

void cgs_sisd::store( value_t& lhs, value_t const& rhs ){
	Value* src = rhs.load( lhs.abi() );
	Value* address = NULL;
	value_kinds kind = lhs.kind();

	if( kind == vkind_ref ){	
		address = lhs.raw();
	} else if ( kind == vkind_swizzle ){
		if( is_vector( lhs.parent()->hint()) ){
			assert( lhs.parent()->storable() );
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else {
			address = lhs.load_ref();
		}
	}

	StoreInst* inst = builder().CreateStore( src, address );
	inst->setAlignment(4);
}

value_t cgs_sisd::cast_ints( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_sisd::cast_i2f( value_t const& v, value_tyinfo* dest_tyi )
{
	builtin_types hint_i = v.hint();
	builtin_types hint_f = dest_tyi->hint();

	Value* val = NULL;
	if( is_signed(hint_i) ){
		val = builder().CreateSIToFP( v.load(), dest_tyi->ty(abi_llvm) );
	} else {
		val = builder().CreateUIToFP( v.load(), dest_tyi->ty(abi_llvm) );
	}

	return create_value( dest_tyi, builtin_types::none, val, vkind_value, abi_llvm );
}

value_t cgs_sisd::cast_f2i( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_sisd::cast_f2f( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_sisd::null_value( value_tyinfo* tyinfo, abis abi )
{
	assert( tyinfo && abi != abi_unknown );
	Type* value_type = tyinfo->ty(abi);
	assert( value_type );
	return create_value( tyinfo, Constant::getNullValue(value_type), vkind_value, abi );
}

value_t cgs_sisd::null_value( builtin_types bt, abis abi )
{
	assert( bt != builtin_types::none );
	Type* valty = type_( bt, abi );
	value_t val = create_value( bt, Constant::getNullValue( valty ), vkind_value, abi );
	return val;
}

value_t cgs_sisd::create_vector( std::vector<value_t> const& scalars, abis abi ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

void cgs_sisd::emit_return(){
	builder().CreateRetVoid();
}

void cgs_sisd::emit_return( value_t const& ret_v, abis abi ){
	if( abi == abi_unknown ){ abi = fn().abi(); }

	if( fn().first_arg_is_return_address() ){
		builder().CreateStore( ret_v.load(abi), fn().return_address() );
		builder().CreateRetVoid();
	} else {
		builder().CreateRet( ret_v.load(abi) );
	}
}

void cgs_sisd::push_fn( function_t const& fn ){
	fn_ctxts.push_back(fn);
}

void cgs_sisd::pop_fn(){
	fn_ctxts.pop_back();
}

function_t cgs_sisd::fetch_function( shared_ptr<function_type> const& fn_node ){

	cgllvm_sctxt* fn_ctxt = node_ctxt( fn_node, false );
	if( fn_ctxt->data().self_fn ){
		return fn_ctxt->data().self_fn;
	}

	function_t ret;
	ret.fnty = fn_node.get();
	ret.c_compatible = fn_node->si_ptr<storage_si>()->c_compatible();

	abis abi = ret.c_compatible ? abi_c : abi_llvm;

	vector<Type*> par_tys;

	Type* ret_ty = node_ctxt( fn_node->retval_type, false )->get_typtr()->ty( abi );

	ret.ret_void = true;
	if( abi == abi_c ){
		if( fn_node->retval_type->tycode != builtin_types::_void ){
			// If function need C compatible and return value is not void, The first parameter is set to point to return value, and parameters moves right.
			Type* ret_ptr = PointerType::getUnqual( ret_ty );
			par_tys.push_back( ret_ptr );
			ret.ret_void = false;
		}

		ret_ty = Type::getVoidTy( context() );
	}

	// Create function type.
	BOOST_FOREACH( shared_ptr<parameter> const& par, fn_node->params )
	{
		cgllvm_sctxt* par_ctxt = node_ctxt( par, false );
		value_tyinfo* par_ty = par_ctxt->get_typtr();
		assert( par_ty );

		//		bool is_ref = par->si_ptr<storage_si>()->is_reference();

		Type* par_llty = par_ty->ty( abi ); 
		if( ret.c_compatible && !is_scalar(par_ty->hint()) ){
			par_tys.push_back( PointerType::getUnqual( par_llty ) );
		} else {
			par_tys.push_back( par_llty );
		}
	}


	FunctionType* fty = FunctionType::get( ret_ty, par_tys, false );

	// Create function
	ret.fn = Function::Create( fty, Function::ExternalLinkage, fn_node->symbol()->mangled_name(), module() );

	ret.cg = this;
	return ret;
}


void cgs_sisd::clean_empty_blocks()
{
	assert( in_function() );

	typedef Function::BasicBlockListType::iterator block_iterator_t;
	block_iterator_t beg = fn().fn->getBasicBlockList().begin();
	block_iterator_t end = fn().fn->getBasicBlockList().end();

	dbg_print_blocks( fn().fn );

	// Remove useless blocks
	vector<BasicBlock*> useless_blocks;

	for( block_iterator_t it = beg; it != end; ++it )
	{
		// If block has terminator, that's a well-formed block.
		if( it->getTerminator() ) continue;

		// Add no-pred & empty block to remove list.
		if( llvm::pred_begin(it) == llvm::pred_end(it) && it->empty() ){
			useless_blocks.push_back( it );
			continue;
		}
	}

	BOOST_FOREACH( BasicBlock* bb, useless_blocks ){
		bb->removeFromParent();
	}

	// Relink unlinked blocks
	beg = fn().fn->getBasicBlockList().begin();
	end = fn().fn->getBasicBlockList().end();
	for( block_iterator_t it = beg; it != end; ++it ){
		if( it->getTerminator() ) continue;

		// Link block to next.
		block_iterator_t next_it = it;
		++next_it;
		builder().SetInsertPoint( it );
		if( next_it != fn().fn->getBasicBlockList().end() ){
			builder().CreateBr( next_it );
		} else {
			if( !fn().fn->getReturnType()->isVoidTy() ){
				builder().CreateRet( Constant::getNullValue( fn().fn->getReturnType() ) );
			} else {
				emit_return();
			}
		}
	}
}

value_t cgs_sisd::create_scalar( Value* val, value_tyinfo* tyinfo ){
	return create_value( tyinfo, val, vkind_value, abi_llvm );
}

sasl::code_generator::value_t cgs_sisd::emit_mul( value_t const& lhs, value_t const& rhs )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	if( is_scalar(lhint) ){
		if( is_scalar(rhint) ){
			return emit_mul_ss_vv( lhs, rhs );
		} else if ( is_vector(rhint) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else if ( is_matrix(rhint) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	} else if ( is_vector(lhint) ){
		if( is_scalar(rhint) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else if ( is_vector(rhint) ){
			emit_mul_ss_vv( lhs, rhs );
		} else if ( is_matrix(rhint) ){
			return emit_mul_vm( lhs, rhs );
		}
	} else if ( is_matrix(lhint) ){
		if( is_scalar(rhint) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else if( is_vector(rhint) ){
			return emit_mul_mv( lhs, rhs );
		} else if( is_matrix(rhint) ){
			return emit_mul_mm( lhs, rhs );
		}
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

sasl::code_generator::value_t cgs_sisd::emit_add( value_t const& lhs, value_t const& rhs )
{
	builtin_types hint = lhs.hint();

	assert( hint != builtin_types::none );
	assert( is_scalar( scalar_of( hint ) ) );
	assert( hint == rhs.hint() );

	if( is_scalar(hint) || is_vector(hint) ){
		return emit_add_ss_vv(lhs, rhs);
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_sisd::emit_add_ss_vv( value_t const& lhs, value_t const& rhs )
{
	EMIT_OP_SS_VV_BODY(Add);
}

void cgs_sisd::set_insert_point( insert_point_t const& ip ){
	builder().SetInsertPoint(ip.block);
}

value_t cgs_sisd::emit_dot( value_t const& lhs, value_t const& rhs )
{
	return emit_dot_vv(lhs, rhs);
}

value_t cgs_sisd::emit_dot_vv( value_t const& lhs, value_t const& rhs )
{
	size_t vec_size = vector_size( lhs.hint() );

	value_t total = null_value( scalar_of( lhs.hint() ), abi_llvm );

	for( size_t i = 0; i < vec_size; ++i ){
		value_t lhs_elem = emit_extract_elem( lhs, i );
		value_t rhs_elem = emit_extract_elem( rhs, i );

		value_t elem_mul = emit_mul_ss_vv( lhs_elem, rhs_elem );
		total.emplace( emit_add_ss_vv( total, elem_mul ).to_rvalue() );
	}

	return total;
}

value_t cgs_sisd::emit_extract_ref( value_t const& lhs, int idx )
{
	assert( lhs.storable() );

	builtin_types agg_hint = lhs.hint();

	if( is_vector(agg_hint) ){
		char indexes[4] = { (char)idx, -1, -1, -1 };
		uint32_t mask = indexes_to_mask( indexes );
		return value_t::slice( lhs, mask );
	} else if( is_matrix(agg_hint) ){
		EFLIB_ASSERT_UNIMPLEMENTED();
		return value_t();
	} else if ( agg_hint == builtin_types::none ){
		Value* agg_address = lhs.load_ref();
		Value* elem_address = builder().CreateStructGEP( agg_address, (unsigned)idx );
		value_tyinfo* tyinfo = NULL;
		if( lhs.tyinfo() ){
			tyinfo = member_tyinfo( lhs.tyinfo(), (size_t)idx );
		}
		return create_value( tyinfo, elem_address, vkind_ref, lhs.abi() );
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_sisd::emit_extract_ref( value_t const& lhs, value_t const& idx )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_sisd::emit_extract_val( value_t const& lhs, int idx )
{
	builtin_types agg_hint = lhs.hint();

	Value* val = lhs.load();
	Value* elem_val = NULL;
	abis abi = abi_unknown;

	builtin_types elem_hint = builtin_types::none;
	value_tyinfo* elem_tyi = NULL;

	if( agg_hint == builtin_types::none ){
		elem_val = builder().CreateExtractValue(val, static_cast<unsigned>(idx));
		abi = lhs.abi();
		elem_tyi = member_tyinfo( lhs.tyinfo(), (size_t)idx );
	} else if( is_scalar(agg_hint) ){
		assert( idx == 0 );
		elem_val = val;
		elem_hint = agg_hint;
	} else if( is_vector(agg_hint) ){
		switch( lhs.abi() ){
		case abi_c:
			elem_val = builder().CreateExtractValue(val, static_cast<unsigned>(idx));
			break;
		case abi_llvm:
			elem_val = builder().CreateExtractElement(val, int_(idx) );
			break;
		default:
			assert(!"Unknown ABI");
			break;
		}
		abi = abi_llvm;
		elem_hint = scalar_of(agg_hint);
	} else if( is_matrix(agg_hint) ){
		elem_val = builder().CreateExtractValue(val, static_cast<unsigned>(idx));
		abi = lhs.abi();
		elem_hint = vector_of( scalar_of(agg_hint), vector_size(agg_hint) );
	}

	// assert( elem_tyi || elem_hint != builtin_types::none );

	return create_value( elem_tyi, elem_hint, elem_val, vkind_value, abi );
}

value_t cgs_sisd::emit_extract_val( value_t const& lhs, value_t const& idx )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_sisd::emit_mul_ss_vv( value_t const& lhs, value_t const& rhs )
{
	EMIT_OP_SS_VV_BODY(Mul);
}

value_t cgs_sisd::emit_call( function_t const& fn, vector<value_t> const& args )
{
	vector<Value*> arg_values;
	value_t var;
	if( fn.c_compatible ){
		// 
		if ( fn.first_arg_is_return_address() ){
			var = create_variable( fn.get_return_ty().get(), abi_c, "ret" );
			arg_values.push_back( var.load_ref() );
		}

		BOOST_FOREACH( value_t const& arg, args ){
			builtin_types hint = arg.hint();
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

	Value* ret_val = builder().CreateCall( fn.fn, arg_values );

	if( fn.first_arg_is_return_address() ){
		return var;
	}

	abis ret_abi = fn.c_compatible ? abi_c : abi_llvm;
	return create_value( fn.get_return_ty().get(), ret_val, vkind_value, ret_abi );
}

value_t cgs_sisd::emit_insert_val( value_t const& lhs, value_t const& idx, value_t const& elem_value )
{
	Value* indexes[1] = { idx.load() };
	Value* agg = lhs.load();
	Value* new_value = NULL;
	if( agg->getType()->isStructTy() ){
		assert(false);
	} else if ( agg->getType()->isVectorTy() ){
		new_value = builder().CreateInsertElement( agg, elem_value.load(), indexes[0] );
	}
	assert(new_value);
	
	return create_value( lhs.tyinfo(), lhs.hint(), new_value, vkind_value, lhs.abi() );
}

value_t cgs_sisd::emit_insert_val( value_t const& lhs, int index, value_t const& elem_value )
{
	Value* agg = lhs.load();
	Value* new_value = NULL;
	if( agg->getType()->isStructTy() ){
		new_value = builder().CreateInsertValue( agg, elem_value.load(lhs.abi()), (unsigned)index );
	} else if ( agg->getType()->isVectorTy() ){
		value_t index_value = create_value( builtin_types::_sint32, int_(index), vkind_value, abi_llvm );
		return emit_insert_val( lhs, index_value, elem_value );
	}
	assert(new_value);

	return create_value( lhs.tyinfo(), lhs.hint(), new_value, vkind_value, lhs.abi() );
}

value_t cgs_sisd::emit_mul_mv( value_t const& lhs, value_t const& rhs )
{
	builtin_types mhint = lhs.hint();
	builtin_types vhint = rhs.hint();

	size_t row_count = vector_count(mhint);
	size_t vec_size = vector_size(mhint);

	builtin_types ret_hint = vector_of( scalar_of(vhint), row_count );

	value_t ret_v = null_value( ret_hint, lhs.abi() );
	for( size_t irow = 0; irow < row_count; ++irow ){
		value_t row_vec = emit_extract_val( lhs, irow );
		ret_v = emit_insert_val( ret_v, irow, emit_dot_vv(row_vec, rhs) );
	}

	return ret_v;
}

value_t cgs_sisd::emit_mul_mm( value_t const& lhs, value_t const& rhs )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	size_t out_v = vector_size( lhint );
	size_t out_r = vector_count( rhint );
	size_t inner_size = vector_size(rhint);

	builtin_types out_row_hint = vector_of( scalar_of(lhint), out_v );
	builtin_types out_hint = matrix_of( scalar_of(lhint), out_v, out_r );
	abis out_abi = lhs.abi();

	vector<value_t> out_cells(out_v*out_r);
	out_cells.resize( out_v*out_r );

	// Caluclate matrix cells.
	for( size_t icol = 0; icol < out_v; ++icol){
		value_t col = emit_extract_col( rhs, icol );
		for( size_t irow = 0; irow < out_r; ++irow )
		{
			value_t row = emit_extract_col( rhs, icol );
			out_cells[irow*out_v+icol] = emit_dot_vv( col, row );
		}
	}

	// Compose cells to matrix
	value_t ret_value = null_value( out_hint, out_abi );
	for( size_t irow = 0; irow < out_r; ++irow ){
		value_t row_vec = null_value( out_row_hint, out_abi );
		for( size_t icol = 0; icol < out_v; ++icol ){
			row_vec = emit_insert_val( row_vec, (int)icol, out_cells[irow*out_v+icol] );
		}
		ret_value = emit_insert_val( ret_value, (int)irow, row_vec );
	}

	return ret_value;
}

value_t cgs_sisd::emit_extract_col( value_t const& lhs, size_t index )
{
	value_t val = lhs.to_rvalue();
	builtin_types mat_hint( lhs.hint() );
	assert( is_matrix(mat_hint) );

	size_t row_count = vector_count( mat_hint );

	builtin_types out_hint = vector_of( scalar_of(mat_hint), row_count );

	value_t out_value = null_value( out_hint, lhs.abi() );
	for( size_t irow = 0; irow < row_count; ++irow ){
		value_t row = emit_extract_val( val, (int)irow );
		value_t cell = emit_extract_val( row, (int)index );
		out_value = emit_insert_val( out_value, (int)irow, cell );
	}

	return out_value;
}

value_t cgs_sisd::emit_mul_vm( value_t const& lhs, value_t const& rhs )
{
	size_t out_v = vector_size( rhs.hint() );

	value_t lrv = lhs.to_rvalue();
	value_t rrv = rhs.to_rvalue();

	value_t ret = null_value( vector_of( scalar_of(lhs.hint()), out_v ), lhs.abi() );
	for( size_t idx = 0; idx < out_v; ++idx ){
		ret = emit_insert_val( ret, (int)idx, emit_dot_vv( lrv, emit_extract_col(rrv, idx) ) );
	}

	return ret;
}

value_tyinfo* cgs_sisd::member_tyinfo( value_tyinfo const* agg, size_t index ) const
{
	if( !agg ){
		return NULL;
	} else if ( agg->tyn_ptr()->is_struct() ){
		shared_ptr<struct_type> struct_sty = agg->tyn_ptr()->as_handle<struct_type>();

		size_t var_index = 0;
		BOOST_FOREACH( shared_ptr<declaration> const& child, struct_sty->decls ){
			if( child->node_class() == node_ids::variable_declaration ){
				shared_ptr<variable_declaration> vardecl = child->as_handle<variable_declaration>();
				var_index += vardecl->declarators.size();
				if( index < var_index ){
					return const_cast<cgs_sisd*>(this)->node_ctxt( vardecl, false )->get_typtr();
				}
			}
		}

		assert(!"Out of struct bound.");
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return NULL;
}

value_t cgs_sisd::emit_extract_elem_mask( value_t const& vec, uint32_t mask )
{
	char indexes[4] = {-1, -1, -1, -1};
	mask_to_indexes( indexes, mask );
	assert( indexes[0] != -1 );
	if( indexes[1] == -1 ){
		return emit_extract_elem( vec, indexes[0] );
	} else {
		// Caculate out size
		/*int out_size = 0;
		for( int i = 0; i < 4; ++i ){
		if( indexes[i] == -1 ){ break; }
		++out_size;
		}*/

		EFLIB_ASSERT_UNIMPLEMENTED();

		return value_t();
	}
}

value_t cgs_sisd::emit_sub_ss_vv( value_t const& lhs, value_t const& rhs )
{
	EMIT_OP_SS_VV_BODY( Sub );
}

value_t cgs_sisd::emit_sub( value_t const& lhs, value_t const& rhs )
{
	builtin_types hint = lhs.hint();

	assert( hint != builtin_types::none );
	assert( is_scalar( scalar_of( hint ) ) );
	assert( hint == rhs.hint() );

	if( is_scalar(hint) || is_vector(hint) ){
		return emit_sub_ss_vv(lhs, rhs);
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

insert_point_t cgs_sisd::insert_point() const
{
	insert_point_t ret;
	ret.block = builder().GetInsertBlock();
	return ret;
}

void cgs_sisd::jump_to( insert_point_t const& ip )
{
	assert( ip );
	if( !insert_point().block->getTerminator() ){
		builder().CreateBr( ip.block );
	}
}

void cgs_sisd::jump_cond( value_t const& cond_v, insert_point_t const const & true_ip, insert_point_t const& false_ip )
{
	Value* cond = cond_v.load();
	builder().CreateCondBr( cond, true_ip.block, false_ip.block );
}


Value* cgs_sisd::select_( Value* cond, Value* yes, Value* no )
{
	return builder().CreateSelect( cond, yes, no );
}

value_t cgs_sisd::emit_cmp_eq( value_t const& lhs, value_t const& rhs )
{
	EMIT_CMP_EQ_NE_BODY( EQ );
}

value_t cgs_sisd::emit_cmp_lt( value_t const& lhs, value_t const& rhs )
{
	EMIT_CMP_BODY( LT );
}

value_t cgs_sisd::emit_cmp_le( value_t const& lhs, value_t const& rhs )
{
	EMIT_CMP_BODY( LE );
}

value_t cgs_sisd::emit_cmp_ne( value_t const& lhs, value_t const& rhs )
{
	EMIT_CMP_EQ_NE_BODY( NE );
}

value_t cgs_sisd::emit_cmp_ge( value_t const& lhs, value_t const& rhs )
{
	EMIT_CMP_BODY( GE );
}

value_t cgs_sisd::emit_cmp_gt( value_t const& lhs, value_t const& rhs )
{
	EMIT_CMP_BODY( GT );
}

value_t cgs_sisd::emit_sqrt( value_t const& arg_value )
{
	builtin_types hint = arg_value.hint();
	builtin_types scalar_hint = scalar_of( arg_value.hint() );
	if( is_scalar(hint) ){
		if( hint == builtin_types::_float ){
			if( prefer_externals() ) {
				EFLIB_ASSERT_UNIMPLEMENTED();
				//	function_t fn = external_proto( &externals::sqrt_f );
				//	vector<value_t> args;
				//	args.push_back(lhs);
				//	return emit_call( fn, args );
			} else if( support_feature( cpu_sse2 ) && !prefer_scalar_code() ){
				// Extension to 4-elements vector.
				value_t v4 = undef_value( vector_of(scalar_hint, 4), abi_llvm );
				v4 = emit_insert_val( v4, 0, arg_value );
				Value* v = builder().CreateCall( intrin_( Intrinsic::x86_sse_sqrt_ss ), v4.load() );
				Value* ret = builder().CreateExtractElement( v, int_(0) );

				return create_value( arg_value.tyinfo(), hint, ret, vkind_value, abi_llvm );
			} else {
				// Emit LLVM intrinsics
				Value* v = builder().CreateCall( intrin_<float(float)>(Intrinsic::sqrt), arg_value.load() );
				return create_value( arg_value.tyinfo(), arg_value.hint(), v, vkind_value, abi_llvm );
			}
		} else if( hint == builtin_types::_double ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} 
	} else if( is_vector(hint) ) {

		size_t vsize = vector_size(hint);

		if( scalar_hint == builtin_types::_float ){
			if( prefer_externals() ){
				EFLIB_ASSERT_UNIMPLEMENTED();
			} else if( support_feature(cpu_sse2) && !prefer_scalar_code() ){
				// TODO emit SSE2 instrinsic directly.

				// expanded to vector 4
				value_t v4;
				if( vsize == 4 ){	
					v4 = create_value( arg_value.tyinfo(), hint, arg_value.load(abi_llvm), vkind_value, abi_llvm );
				} else {
					v4 = null_value( vector_of( scalar_hint, 4 ), abi_llvm );
					for ( size_t i = 0; i < vsize; ++i ){
						v4 = emit_insert_val( v4, i, emit_extract_elem(arg_value, i) );
					}
				}

				// calculate
				Value* v = builder().CreateCall( intrin_( Intrinsic::x86_sse_sqrt_ps ), v4.load() );

				if( vsize < 4 ){
					// Shrink
					static int indexes[4] = {0, 1, 2, 3};
					Value* mask = vector_( &indexes[0], vsize );
					v = builder().CreateShuffleVector( v, UndefValue::get( v->getType() ), mask );
				}

				return create_value( NULL, hint, v, vkind_value, abi_llvm );
			} else {
				value_t ret = null_value( hint, arg_value.abi() );
				for( size_t i = 0; i < vsize; ++i ){
					value_t elem = emit_extract_elem( arg_value, i );
					ret = emit_insert_val( ret, i, emit_sqrt( arg_value ) );
				}
				return ret;
			}
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return value_t();
}

bool cgs_sisd::prefer_externals() const
{
	return false;
}

bool cgs_sisd::prefer_scalar_code() const
{
	return false;
}

Function* cgs_sisd::intrin_( int v )
{
	return intrins.get( llvm::Intrinsic::ID(v), module() );
}

value_t cgs_sisd::undef_value( builtin_types bt, abis abi )
{
	assert( bt != builtin_types::none );
	Type* valty = type_( bt, abi );
	value_t val = create_value( bt, UndefValue::get(valty), vkind_value, abi );
	return val;
}

value_t cgs_sisd::emit_cross( value_t const& lhs, value_t const& rhs )
{
	assert( lhs.hint() == vector_of( builtin_types::_float, 3 ) );
	assert( rhs.hint() == lhs.hint() );

	int swz_a[] = {1, 2, 0};
	int swz_b[] = {2, 0, 1};

	ConstantVector* swz_va = vector_( swz_a, 3 );
	ConstantVector* swz_vb = vector_( swz_b, 3 );

	Value* lvec_value = lhs.load(abi_llvm);
	Value* rvec_value = rhs.load(abi_llvm);

	Value* lvec_a = builder().CreateShuffleVector( lvec_value, UndefValue::get( lvec_value->getType() ), swz_va );
	Value* lvec_b = builder().CreateShuffleVector( lvec_value, UndefValue::get( lvec_value->getType() ), swz_vb );
	Value* rvec_a = builder().CreateShuffleVector( rvec_value, UndefValue::get( rvec_value->getType() ), swz_va );
	Value* rvec_b = builder().CreateShuffleVector( rvec_value, UndefValue::get( rvec_value->getType() ), swz_vb );

	Value* mul_first = builder().CreateFMul( lvec_a, rvec_b );
	Value* mul_second = builder().CreateFMul( lvec_b, rvec_a );

	Value* ret = builder().CreateFSub( mul_first, mul_second );

	return create_value( lhs.tyinfo(), lhs.hint(), ret, vkind_value, abi_llvm );
}

value_t cgs_sisd::emit_swizzle( value_t const& lhs, uint32_t mask )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_sisd::emit_write_mask( value_t const& vec, uint32_t mask )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

void cgs_sisd::switch_to( value_t const& cond, std::vector< std::pair<value_t, insert_point_t> > const& cases, insert_point_t const& default_branch )
{
	Value* v = cond.load();
	SwitchInst* inst = builder().CreateSwitch( v, default_branch.block, static_cast<unsigned>(cases.size()) );
	for( size_t i_case = 0; i_case < cases.size(); ++i_case ){
		inst->addCase( llvm::cast<ConstantInt>( cases[i_case].first.load() ), cases[i_case].second.block );
	}
}

value_t cgs_sisd::cast_i2b( value_t const& v )
{
	assert( is_integer(v.hint()) );
	return emit_cmp_ne( v, null_value( v.hint(), v.abi() ) );
}

value_t cgs_sisd::cast_f2b( value_t const& v )
{
	assert( is_real(v.hint()) );
	return emit_cmp_ne( v, null_value( v.hint(), v.abi() ) );
}

cgllvm_sctxt* cgs_sisd::node_ctxt( shared_ptr<node> const& node, bool create_if_need )
{
	return cg_service::node_ctxt( node.get(), create_if_need );
}

llvm::Value* cgs_sisd::load( value_t const& v )
{
	value_kinds kind = v.kind();
	Value* raw = v.raw();
	uint32_t masks = v.masks();

	assert( kind != vkind_unknown && kind != vkind_tyinfo_only );

	Value* ref_val = NULL;
	if( kind == vkind_ref || kind == vkind_value ){
		ref_val = raw;
	} else if( ( kind & (~vkind_ref) ) == vkind_swizzle ){
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
		Value* agg_v = v.parent()->load();

		if( index_values.size() == 1 ){
			// Only one member we could extract reference.
			ref_val = emit_extract_val( v.parent()->to_rvalue(), index_values[0] ).load();
		} else {
			// Multi-members must be swizzle/writemask.
			assert( (kind & vkind_ref) == 0 );
			if( v.abi() == abi_c ){
				EFLIB_ASSERT_UNIMPLEMENTED();
				return NULL;
			} else {
				Value* mask = vector_( &indexes[0], index_values.size() );
				return builder().CreateShuffleVector( agg_v, UndefValue::get( agg_v->getType() ), mask );
			}
		}
	} else {
		assert(false);
	}

	if( kind & vkind_ref ){
		return builder().CreateLoad( ref_val );
	} else {
		return ref_val;
	}
}

llvm::Value* cgs_sisd::load( value_t const& v, abis abi )
{
	assert( abi != abi_unknown );
	builtin_types hint = v.hint();

	if( abi != v.abi() ){
		if( is_scalar( hint ) ){
			return v.load();
		} else if( is_vector( hint ) ){
			Value* org_value = v.load();

			value_t ret_value = null_value( hint, abi );

			size_t vec_size = vector_size( hint );
			for( size_t i = 0; i < vec_size; ++i ){
				ret_value = emit_insert_val( ret_value, (int)i, emit_extract_elem(v, i) );
			}

			return ret_value.load();
		} else if( is_matrix( hint ) ){
			value_t ret_value = null_value( hint, abi );
			size_t vec_count = vector_count( hint );
			for( size_t i = 0; i < vec_count; ++i ){
				value_t org_vec = emit_extract_val(v, (int)i);
				ret_value = emit_insert_val( ret_value, (int)i, org_vec );
			}

			return ret_value.load();
		} else {
			// NOTE: We assume that, if tyinfo is null and hint is none, it is only the entry of vs/ps. Otherwise, tyinfo must be not NULL.
			if( !v.tyinfo() && hint == builtin_types::none ){
				EFLIB_ASSERT_UNIMPLEMENTED();
			} else {
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
		}
		return NULL;
	} else {
		return v.load();
	}
}

llvm::Value* cgs_sisd::load_ref( value_t const& v )
{
	value_kinds kind = v.kind();

	if( kind == vkind_ref ){
		return v.raw();
	} else if( kind == (vkind_swizzle|vkind_ref) ){
		value_t non_ref( v );
		non_ref.kind( vkind_swizzle );
		return non_ref.load();
	} if( kind == vkind_swizzle ){
		assert( v.masks() );
		return emit_extract_elem_mask( *v.parent(), v.masks() ).load_ref();
	}
	return NULL;
}

abis cgs_sisd::intrinsic_abi() const
{
	return abi_llvm;
}

void function_t::arg_name( size_t index, std::string const& name ){
	size_t param_size = fn->arg_size();
	if( first_arg_is_return_address() ){
		--param_size;
	}
	assert( index < param_size );

	Function::arg_iterator arg_it = fn->arg_begin();
	if( first_arg_is_return_address() ){
		++arg_it;
	}

	for( size_t i = 0; i < index; ++i ){ ++arg_it; }
	arg_it->setName( name );
}

void function_t::args_name( vector<string> const& names )
{
	Function::arg_iterator arg_it = fn->arg_begin();
	vector<string>::const_iterator name_it = names.begin();

	if( first_arg_is_return_address() ){
		arg_it->setName(".ret");
		++arg_it;
	}

	for( size_t i = 0; i < names.size(); ++i ){
		arg_it->setName( *name_it );
		++arg_it;
		++name_it;
	}
}

shared_ptr<value_tyinfo> function_t::get_return_ty() const{
	assert( fnty->is_function() );
	return shared_ptr<value_tyinfo>( cg->node_ctxt( fnty->retval_type.get(), false )->get_tysp() );
}

size_t function_t::arg_size() const{
	assert( fn );
	if( fn ){
		if( first_arg_is_return_address() ){ return fn->arg_size() - 1; }
		return fn->arg_size() - 1;
	}
	return 0;
}

value_t function_t::arg( size_t index ) const
{
	// If c_compatible and not void return, the first argument is address of return value.
	size_t arg_index = index;
	if( first_arg_is_return_address() ){ ++arg_index; }

	shared_ptr<parameter> par = fnty->params[index];
	value_tyinfo* par_typtr = cg->node_ctxt( par.get(), false )->get_typtr();

	Function::ArgumentListType::iterator it = fn->arg_begin();
	for( size_t idx_counter = 0; idx_counter < arg_index; ++idx_counter ){
		++it;
	}

	abis arg_abi = c_compatible ? abi_c: abi_llvm;
	return cg->create_value( par_typtr, &(*it), arg_is_ref(index) ? vkind_ref : vkind_value, arg_abi );
}

function_t::function_t(): fn(NULL), fnty(NULL), ret_void(true), c_compatible(false), cg(NULL)
{
}

bool function_t::arg_is_ref( size_t index ) const{
	assert( index < fnty->params.size() );

	builtin_types hint = fnty->params[index]->si_ptr<storage_si>()->type_info()->tycode;
	return c_compatible && !is_scalar(hint);
}

bool function_t::first_arg_is_return_address() const
{
	return c_compatible && !ret_void;
}

abis function_t::abi() const
{
	return c_compatible ? abi_c : abi_llvm;
}

llvm::Value* function_t::return_address() const
{
	if( first_arg_is_return_address() ){
		return &(*fn->arg_begin());
	}
	return NULL;
}

void function_t::return_name( std::string const& s )
{
	if( first_arg_is_return_address() ){
		fn->arg_begin()->setName( s );
	}
}

void function_t::inline_hint()
{
	fn->addAttribute( 0, llvm::Attribute::InlineHint );
}

insert_point_t::insert_point_t(): block(NULL)
{
}

END_NS_SASL_CODE_GENERATOR();