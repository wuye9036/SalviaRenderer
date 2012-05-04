#include <sasl/include/code_generator/llvm/cgllvm_simd.h>

#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/utility.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>

#include <sasl/enums/enums_utility.h>

#include <salviar/include/shader_abi.h>
#include <eflib/include/math/math.h>

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

using salviar::PACKAGE_ELEMENT_COUNT;
using salviar::PACKAGE_LINE_ELEMENT_COUNT;

using eflib::support_feature;
using eflib::cpu_sse2;
using eflib::ceil_to_pow2;

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
using llvm::CmpInst;

namespace Intrinsic = llvm::Intrinsic;

using boost::any;
using boost::shared_ptr;
using boost::enable_if;
using boost::is_integral;
using boost::unordered_map;
using boost::lexical_cast;

using std::vector;
using std::string;

BEGIN_NS_SASL_CODE_GENERATOR();

void cgs_simd::store( value_t& lhs, value_t const& rhs )
{
	Value* src = rhs.load( lhs.abi() );
	Value* address = NULL;
	value_kinds kind = lhs.kind();

	if( kind == vkind_ref ){	
		address = lhs.raw();

		if( is_scalar( lhs.hint() ) || is_vector( lhs.hint() ) ){
			size_t value_length = is_scalar( lhs.hint() ) ? 1 : vector_size( lhs.hint() );
			size_t padded_value_length = ceil_to_pow2(value_length);
			Value* mask = expanded_mask( padded_value_length );
			Value* dest_value = builder().CreateLoad( address );
			src = builder().CreateSelect( i8toi1_(mask), src, dest_value, "Merged" );
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	} else if ( kind == vkind_swizzle ){
		if( is_vector( lhs.parent()->hint()) )
		{
			assert( lhs.parent()->storable() );
			Value* parent_address = lhs.parent()->load_ref();
			Value* r_value = rhs.load( lhs.abi() );

			char indexes[4];
			mask_to_indexes( indexes, lhs.masks() );
			uint32_t idx_len = indexes_length( indexes );

			switch ( lhs.abi() ){	
			case abi_c:
				{
					for( size_t i_write_idx = 0; i_write_idx < idx_len; ++i_write_idx ){
						value_t element_val = emit_extract_val( rhs, static_cast<int>(i_write_idx) );
						Value* mem_ptr = builder().CreateStructGEP( parent_address, indexes[i_write_idx] );
						builder().CreateStore( element_val.load(), mem_ptr );
					}
					return;
				}
			case abi_llvm:
				{
					value_t parent_v = lhs.parent()->to_rvalue();
					for( size_t i_write_idx = 0; i_write_idx < idx_len; ++i_write_idx ){
						value_t element_val = emit_extract_val( rhs, static_cast<int>(i_write_idx) );
						parent_v = emit_insert_val( rhs, indexes[i_write_idx], element_val );
					}
					lhs.parent()->store( parent_v );
					return;
				}
			case abi_vectorize:
				EFLIB_ASSERT_UNIMPLEMENTED();
				break;
			case abi_package:
				{
					int dst_elem_pitch = ceil_to_pow2( vector_size( lhs.parent()->hint() ) );
					int src_elem_pitch = ceil_to_pow2( vector_size( rhs.hint() ) );

					Value* parent_value = lhs.parent()->load();
					for( size_t i_elem = 0; i_elem < PACKAGE_ELEMENT_COUNT; ++i_elem ){
						for( size_t i_write_idx = 0; i_write_idx < idx_len; ++i_write_idx ){
							int src_index = static_cast<int>( src_elem_pitch*i_elem+i_write_idx );
							int dst_index = static_cast<int>( dst_elem_pitch*i_elem+indexes[i_write_idx] );
					
							Value* new_val = builder().CreateExtractElement( r_value, int_(src_index) );
							Value* old_val = builder().CreateExtractElement( parent_value, int_(dst_index) ); 
							Value* elem_exec_mask = builder().CreateExtractElement( exec_masks.back(), int_( static_cast<int>(i_elem) ) );
							Value* elem_val = builder().CreateSelect( i8toi1_(elem_exec_mask), new_val, old_val );
							parent_value = builder().CreateInsertElement( parent_value, elem_val, int_(dst_index) );
						}
					}
					value_t new_v = *lhs.parent();
					new_v.emplace( parent_value, vkind_value, lhs.abi() );
					lhs.parent()->store( new_v );
					return;
				}
			default:
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
		} else {
			address = lhs.load_ref();
		}
	}

	// Masked Store
	StoreInst* inst = builder().CreateStore( src, address );
	//inst->setAlignment(4);
}

value_t cgs_simd::cast_ints( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_i2f( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_f2i( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_f2f( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_i2b( value_t const& v )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_f2b( value_t const& v )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::create_vector( vector<value_t> const& scalars, abis abi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

abis cgs_simd::intrinsic_abi() const
{
	return abi_vectorize;
}

void cgs_simd::emit_return()
{
	builder().CreateRetVoid();
}

void cgs_simd::emit_return( value_t const& ret_v, abis abi )
{
	if( abi == abi_unknown ){ abi = fn().abi(); }

	if( fn().first_arg_is_return_address() ){
		builder().CreateStore( ret_v.load(abi), fn().return_address() );
		builder().CreateRetVoid();
	} else {
		builder().CreateRet( ret_v.load(abi) );
	}
}

abis cgs_simd::param_abi( bool /*c_compatible*/ ) const
{
	return abi_package;
}

value_t cgs_simd::create_scalar( llvm::Value* v, value_tyinfo* tyi, builtin_types hint )
{
	Type* ty = NULL;
	if( tyi )
	{
		ty = tyi->ty( abi_vectorize );
	}
	else
	{
		ty = type_( hint, abi_vectorize );
	}
	
	Value* vectorize_v = Constant::getNullValue( ty );

	for( size_t i_elem = 0; i_elem < llvm::cast<VectorType>(ty)->getNumElements(); ++i_elem ){
		vectorize_v = builder().CreateInsertElement( vectorize_v, v, int_( static_cast<int>(i_elem) ) );
	}

	return create_value( tyi, hint, vectorize_v, vkind_value, abi_vectorize );
}

void cgs_simd::function_beg()
{
	exec_masks.push_back( all_one_mask() );
	break_masks.push_back(NULL);
	continue_masks.push_back(NULL);
}

void cgs_simd::function_end()
{
	// Do nothing
}

Value* cgs_simd::all_one_mask()
{
	Type* mask_ty = type_( builtin_types::_boolean, abi_package );
	Value* mask_v = Constant::getAllOnesValue(mask_ty);
	return mask_v;
}

Value* cgs_simd::expanded_mask( uint32_t expanded_times )
{
	Value* exec_mask_v = exec_masks.back();
	if( expanded_times == 1 ){
		return exec_mask_v;
	}

	vector<int> shuffle_mask;
	shuffle_mask.reserve( expanded_times * PACKAGE_ELEMENT_COUNT );
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		shuffle_mask.insert( shuffle_mask.end(), expanded_times, i );
	}
	Constant* shuffle_mask_v = vector_( &(shuffle_mask[0]), shuffle_mask.size() );
	return builder().CreateShuffleVector( exec_mask_v, exec_mask_v, shuffle_mask_v );
}

void cgs_simd::if_cond_beg()
{
	// Do nothing
}

void cgs_simd::if_cond_end( value_t const& cond )
{
	cond_exec_masks.push_back( cond.load( abi_package ) );
}

void cgs_simd::then_beg()
{
	Value* then_mask = builder().CreateAnd( cond_exec_masks.back(), exec_masks.back(), "mask.then" );
	exec_masks.push_back( then_mask );
}

void cgs_simd::then_end()
{
	exec_masks.pop_back();
	apply_break_and_continue();
}

void cgs_simd::else_beg()
{
	Value* inv_cond_exec_mask = builder().CreateNot( cond_exec_masks.back(), "cond.inv" );
	Value* else_mask =  builder().CreateAnd( inv_cond_exec_mask, exec_masks.back(), "mask.else" );
	exec_masks.push_back( else_mask );
}

void cgs_simd::else_end()
{
	exec_masks.pop_back();
	apply_break_and_continue();
}

value_t cgs_simd::emit_ddx( value_t const& v )
{
	return derivation( v, slm_vertical );
}

value_t cgs_simd::emit_ddy( value_t const& v )
{
	return derivation( v, slm_horizontal );
}

value_t cgs_simd::derivation( value_t const& v, slice_layout_mode slm )
{
	if( v.abi() != abi_package ){
		return null_value( v.hint(), v.abi() );
	}

	int const PACKAGE_LINES = PACKAGE_ELEMENT_COUNT / PACKAGE_LINE_ELEMENT_COUNT;
	int const MAX_SLICES_COUNT = ( PACKAGE_LINES > PACKAGE_LINE_ELEMENT_COUNT ? PACKAGE_LINES : PACKAGE_LINE_ELEMENT_COUNT );

	Value* pkg_v = v.load();
	Value* diff_v = NULL;

	builtin_types hint = v.hint();
	builtin_types scalar_hint = is_scalar(hint) ? hint : scalar_of(hint);
	
	if( is_scalar(hint) || is_vector(hint) )
	{
		int elem_width = static_cast<int>( hint == scalar_hint ? 1 : vector_size(hint) );
		int padded_elem_width = ceil_to_pow2(elem_width);

		int slice_stride = 0;
		int elem_stride = 0;
		int slice_count = 0;
		int slice_size = 0;
		if( slm == slm_horizontal ){
			slice_stride = 0;
			elem_stride = 1;
			slice_count = PACKAGE_ELEMENT_COUNT / PACKAGE_LINE_ELEMENT_COUNT;
			slice_size = PACKAGE_LINE_ELEMENT_COUNT;
		} else {
			slice_stride = -PACKAGE_ELEMENT_COUNT+1;
			elem_stride = PACKAGE_LINE_ELEMENT_COUNT;
			slice_count = PACKAGE_LINE_ELEMENT_COUNT;
			slice_size = PACKAGE_ELEMENT_COUNT / PACKAGE_LINE_ELEMENT_COUNT;
		}

		Value* slides[MAX_SLICES_COUNT] = {NULL};
		unpack_slices( pkg_v, slice_count, slice_size, slice_stride, elem_stride, padded_elem_width, slides );

		// Compute derivations
		Value* diff[MAX_SLICES_COUNT] = {NULL};
		for( int i = 0; i < slice_count; i+=2 )
		{
			if( is_integer(scalar_hint) ){
				if( is_signed(scalar_hint) ){
					diff[i] = builder().CreateNSWSub( slides[i+1], slides[i] );
				} else {
					diff[i] = builder().CreateNUWSub( slides[i+1], slides[i] );
				}
			} else {
				diff[i] = builder().CreateFSub( slides[i+1], slides[i] );
			}
			diff[i+1] = diff[i];
		}

		diff_v = pack_slices( diff, slice_count, slice_size, slice_stride, elem_stride, padded_elem_width );
	}
	else if( is_matrix(hint) )
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return create_value( v.tyinfo(), v.hint(), diff_v, vkind_value, abi_package );
}

/// Pack slides to vector.
Value* cgs_simd::pack_slices( Value** slices, int slice_count, int slice_size, int slice_stride, int elem_stride, int elem_width )
{
	Type* element_ty = ((VectorType*)slices[0]->getType())->getElementType();
	Value* vec = UndefValue::get( VectorType::get( element_ty, PACKAGE_ELEMENT_COUNT*elem_width ) );
	int index = 0;
	for( int i_slice = 0; i_slice < slice_count; ++i_slice ){
		for( int i_elem = 0; i_elem < slice_size; ++i_elem ){
			for( int i_scalar = 0; i_scalar < elem_width; ++i_scalar ){
				Value* scalar  = builder().CreateExtractElement( slices[i_slice], int_( i_elem * elem_width + i_scalar ) );
				Value* index_v = int_(index+i_scalar);
				vec = builder().CreateInsertElement( vec, scalar, index_v );
			}
			index += ( elem_stride * elem_width );
		}
		index += slice_stride * elem_width;
	}

	return vec;
}

void cgs_simd::unpack_slices( Value* pkg, int slice_count, int slice_size, int slice_stride, int elem_stride, int elem_width, Value** out_slices )
{
	vector<int> slice_indexes( slice_size*elem_width, 0 );

	int index = 0;
	for( int i_slice = 0; i_slice < slice_count; ++i_slice ){

		// Compute slice indexes
		for( int i_elem = 0; i_elem < slice_size; ++i_elem ){
			for( int i_scalar = 0; i_scalar < elem_width; ++i_scalar ){
				slice_indexes[i_elem*elem_width+i_scalar] = ( index+i_scalar );
			}
			index += ( elem_stride * elem_width );
		}

		// Extract slices
		Constant* slice_indexes_v = vector_( &slice_indexes[0], slice_size*elem_width );
		out_slices[i_slice] = builder().CreateShuffleVector( pkg, pkg, slice_indexes_v );

		index += slice_stride * elem_width;
	}
}

void cgs_simd::for_init_beg() {	enter_loop(); }
void cgs_simd::for_init_end() {}
void cgs_simd::for_cond_beg() {}
void cgs_simd::for_cond_end( value_t const& cond ) { apply_loop_condition( cond ); }
void cgs_simd::for_body_beg(){}
void cgs_simd::for_body_end(){}
void cgs_simd::for_iter_beg(){}
void cgs_simd::for_iter_end(){ save_next_iteration_exec_mask(); exit_loop(); }

llvm::Value* cgs_simd::all_zero_mask()
{
	Type* mask_ty = type_( builtin_types::_boolean, abi_package );
	Value* mask_v = Constant::getNullValue(mask_ty);
	return mask_v;
}

void cgs_simd::apply_break_and_continue()
{
	apply_break();
	apply_continue();
}

void cgs_simd::if_beg(){}

void cgs_simd::if_end()
{
	cond_exec_masks.pop_back();
}

void cgs_simd::apply_break()
{
	Value* mask = exec_masks.back();
	if( break_masks.back() ){
		mask = builder().CreateAnd( builder().CreateNot( break_masks.back() ), mask );
	}
	exec_masks.back() = mask;
}

void cgs_simd::apply_continue()
{
	Value* mask = exec_masks.back();
	if( continue_masks.back() ){
		mask = builder().CreateAnd( builder().CreateNot( continue_masks.back() ), mask );
	}
	exec_masks.back() = mask;
}

void cgs_simd::break_()
{
	Value* break_mask = break_masks.back();
	if( break_mask == NULL ){
		break_mask = exec_masks.back();
	} else {
		break_mask = builder().CreateOr( exec_masks.back(), break_mask );
	}
	break_masks.back() = break_mask;
	apply_break();
}

void cgs_simd::continue_()
{
	Value* continue_mask = continue_masks.back();
	if( continue_mask == NULL ){
		continue_mask = exec_masks.back();
	} else {
		continue_mask = builder().CreateOr( exec_masks.back(), continue_mask );
	}
	continue_masks.back() = continue_mask;
	apply_continue();
}

value_t cgs_simd::joinable()
{
	Value* v = exec_masks.back();
	Value* ret_bool = builder().CreateExtractElement( v, int_(0) );
	for( int i = 1; i < PACKAGE_ELEMENT_COUNT; ++i ){
		ret_bool = builder().CreateOr( ret_bool, builder().CreateExtractElement( v, int_(i) ) );
	}
	return create_value( builtin_types::_boolean, ret_bool, vkind_value, abi_llvm );
}

void cgs_simd::while_beg(){ enter_loop(); }
void cgs_simd::while_end(){ exit_loop(); }
void cgs_simd::while_cond_beg(){}
void cgs_simd::while_cond_end( value_t const& cond )
{
	apply_loop_condition(cond);
}
void cgs_simd::while_body_beg() {}
void cgs_simd::while_body_end() { save_next_iteration_exec_mask(); }

void cgs_simd::enter_loop()
{
	// TODO
	//  store <16 x i1>, <16 x i1>* var is crashed by LLVM bug.
	// We ext to i8 array and store.
	mask_vars.push_back( builder().CreateAlloca( type_(builtin_types::_uint8, abi_package), NULL, ".for.mask.tmpvar" ) );
	save_loop_execution_mask( exec_masks.back() );

	exec_masks.push_back( exec_masks.back() );
	break_masks.push_back(NULL);
	continue_masks.push_back(NULL);
}

void cgs_simd::exit_loop()
{
	exec_masks.pop_back();
	break_masks.pop_back();
	continue_masks.pop_back();
}

void cgs_simd::apply_loop_condition( value_t const& cond )
{
	Value* exec_mask = load_loop_execution_mask();
	if( cond.abi() != abi_unknown ){
		Value* cond_exec_mask = cond.load( abi_package );
		exec_mask = builder().CreateAnd( exec_mask, cond_exec_mask );
	}

	exec_masks.back() = exec_mask;
	break_masks.back() = NULL;
	continue_masks.back() = NULL;
}

void cgs_simd::do_beg(){ enter_loop(); }
void cgs_simd::do_end(){ exit_loop(); }
void cgs_simd::do_body_beg(){ exec_masks.back() = load_loop_execution_mask(); }
void cgs_simd::do_body_end(){ save_next_iteration_exec_mask(); }
void cgs_simd::do_cond_beg(){}
void cgs_simd::do_cond_end( value_t const& cond ) {	
	apply_loop_condition( cond );
	save_loop_execution_mask( exec_masks.back() );
}

void cgs_simd::save_next_iteration_exec_mask()
{
	Value* next_iter_exec_mask = exec_masks.back();
	if( continue_masks.back() ){
		next_iter_exec_mask = builder().CreateOr( exec_masks.back(), continue_masks.back() );
	}
	save_loop_execution_mask(next_iter_exec_mask);
}

llvm::Value* cgs_simd::load_loop_execution_mask()
{
	Value* mask_as_uchar = builder().CreateLoad( mask_vars.back() );
	return builder().CreateTruncOrBitCast( mask_as_uchar, type_(builtin_types::_boolean, abi_package) );
}

void cgs_simd::save_loop_execution_mask( Value* mask )
{
	Value* mask_as_uchar = builder().CreateZExtOrBitCast( mask, type_( builtin_types::_uint8, abi_package) );
	builder().CreateStore( mask_as_uchar, mask_vars.back() );
}

value_t cgs_simd::packed_mask()
{
	assert( PACKAGE_ELEMENT_COUNT == 16 );

	Value* mask_vec = exec_masks.back();

	Type* packed_mask_ty = type_( builtin_types::_sint16, abi_llvm);
	Value* ret = Constant::getNullValue( packed_mask_ty );
	for( size_t i_mask = 0; i_mask < PACKAGE_ELEMENT_COUNT; ++i_mask ){
		Value* mask_bit = builder().CreateExtractElement( mask_vec, int_<int32_t>(i_mask) );
		mask_bit = builder().CreateZExt( mask_bit, packed_mask_ty );
		ret = builder().CreateShl( ret, 1 );
		ret = builder().CreateOr( ret, mask_bit );
	}

	return create_value( NULL, builtin_types::_sint16, ret, vkind_value, abi_llvm );
}

value_t cgs_simd::emit_and( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_or( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

END_NS_SASL_CODE_GENERATOR();
