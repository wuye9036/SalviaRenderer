#include <sasl/include/codegen/cg_simd.h>

#include <sasl/include/codegen/cg_contexts.h>
#include <sasl/include/codegen/utility.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/symbol.h>

#include <sasl/enums/enums_utility.h>

#include <salviar/include/shader_abi.h>
#include <eflib/include/math/math.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/IRBuilder.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Intrinsics.h>
#include <llvm/TypeBuilder.h>
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

using sasl::semantic::node_semantic;
using sasl::semantic::node_semantic;

using salviar::PACKAGE_ELEMENT_COUNT;
using salviar::PACKAGE_LINE_ELEMENT_COUNT;

using eflib::support_feature;
using eflib::cpu_sse2;
using eflib::ceil_to_pow2;

using namespace sasl::utility;

using llvm::ArrayRef;
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

BEGIN_NS_SASL_CODEGEN();

void cgs_simd::store( multi_value& lhs, multi_value const& rhs )
{
	value_array src = rhs.load( lhs.abi() );
	value_array address(src.size(), NULL);
	value_kinds::id kind = lhs.kind();

	if( kind == value_kinds::reference ){	
		address = lhs.raw();

		if( is_scalar( lhs.hint() ) || is_vector( lhs.hint() ) ){
			Value* mask = exec_masks.back();
			value_array dest_value = ext_->load(address);

			for(uint32_t value_index = 0; value_index < parallel_factor_; ++value_index)
			{
				assert(false);
			}
			value_array selected = ext_->select(mask, src, dest_value);
			src = selected;
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	} else if ( kind == value_kinds::elements ){
		if( is_vector( lhs.parent()->hint()) )
		{
			assert( lhs.parent()->storable() );
			value_array parent_address = lhs.parent()->load_ref();
			value_array r_value = rhs.load( lhs.abi() );

			char indexes[4];
			mask_to_indexes( indexes, lhs.masks() );
			uint32_t idx_len = indexes_length( indexes );

			switch ( lhs.abi() ){	
			case abis::c:
				{
					for( size_t i_write_idx = 0; i_write_idx < idx_len; ++i_write_idx )
					{
						multi_value element_val = emit_extract_val( rhs, static_cast<int>(i_write_idx) );
						value_array mem_ptr = ext_->struct_gep(parent_address, indexes[i_write_idx]);
						ext_->store(element_val.load(), mem_ptr);
					}
					return;
				}
			case abis::llvm:
				{
					multi_value parent_v = lhs.parent()->to_rvalue();
					for( size_t i_write_idx = 0; i_write_idx < idx_len; ++i_write_idx ){
						multi_value element_val = emit_extract_val( rhs, static_cast<int>(i_write_idx) );
						parent_v = emit_insert_val( rhs, indexes[i_write_idx], element_val );
					}
					lhs.parent()->store( parent_v );
					return;
				}
			default:
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
		} else {
			address = lhs.load_ref();
		}
	}

	ext_->store(src, address);
}

multi_value cgs_simd::cast_ints( multi_value const& v, cg_type* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cgs_simd::cast_i2f( multi_value const& v, cg_type* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cgs_simd::cast_f2i( multi_value const& v, cg_type* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cgs_simd::cast_f2f( multi_value const& v, cg_type* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cgs_simd::cast_i2b( multi_value const& v )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cgs_simd::cast_f2b( multi_value const& v )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cgs_simd::create_vector( vector<multi_value> const& scalars, abis::id abi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

void cgs_simd::emit_return()
{
	builder().CreateRetVoid();
}

void cgs_simd::emit_return( multi_value const& ret_v, abis::id abi )
{
	if( abi == abis::unknown ){ abi = fn().abi(); }

	value_array ret_value = ret_v.load(abi);
	if( fn().return_via_arg() ){
		ext_->store( ret_value, fn().return_address() );
		builder().CreateRetVoid();
	} else {
		builder().CreateRet( ext_->get_array(ret_value) );
	}
}

void cgs_simd::function_body_beg()
{
	cg_service::function_body_beg();
	exec_masks.push_back( all_one_mask() );
	break_masks.push_back(NULL);
	continue_masks.push_back(NULL);
}

void cgs_simd::function_body_end()
{
	cg_service::function_body_end();
	// Do nothing
}

Value* cgs_simd::all_one_mask()
{
	uint64_t mask = (1ULL << parallel_factor_) - 1;
	return ext_->get_int( static_cast<uint32_t>(mask) );
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
	Constant* shuffle_mask_v = ext_->get_vector<int>( ArrayRef<int>(shuffle_mask) );
	return builder().CreateShuffleVector( exec_mask_v, exec_mask_v, shuffle_mask_v );
}

void cgs_simd::if_cond_beg()
{
	// Do nothing
}

void cgs_simd::if_cond_end( multi_value const& cond )
{
	assert(false);
	// cond_exec_masks.push_back( cond.load( abis::package ) );
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

multi_value cgs_simd::emit_ddx( multi_value const& v )
{
	return derivation( v, slm_vertical );
}

multi_value cgs_simd::emit_ddy( multi_value const& v )
{
	return derivation( v, slm_horizontal );
}

multi_value cgs_simd::derivation( multi_value const& v, slice_layout_mode slm )
{
#if 0
	if( v.abi() != abis::package ){
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

	return create_value( v.ty(), v.hint(), diff_v, value_kinds::value, abis::package );
#endif
	return multi_value(0);
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
				Value* scalar  = builder().CreateExtractElement( slices[i_slice], ext_->get_int( i_elem * elem_width + i_scalar ) );
				Value* index_v = ext_->get_int(index+i_scalar);
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
		Constant* slice_indexes_v = ext_->get_vector<int>( ArrayRef<int>(slice_indexes) );
		out_slices[i_slice] = builder().CreateShuffleVector( pkg, pkg, slice_indexes_v );

		index += slice_stride * elem_width;
	}
}

void cgs_simd::for_init_beg() {	enter_loop(); }
void cgs_simd::for_init_end() {}
void cgs_simd::for_cond_beg() {}
void cgs_simd::for_cond_end( multi_value const& cond ) { apply_loop_condition( cond ); }
void cgs_simd::for_body_beg(){}
void cgs_simd::for_body_end(){}
void cgs_simd::for_iter_beg(){}
void cgs_simd::for_iter_end(){ save_next_iteration_exec_mask(); exit_loop(); }

llvm::Value* cgs_simd::all_zero_mask()
{
	assert(false);
	return NULL;
	/*
	Type* mask_ty = type_( builtin_types::_boolean, abis::package );
	Value* mask_v = Constant::getNullValue(mask_ty);
	return mask_v;
	*/
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

multi_value cgs_simd::any_mask_true()
{
	Value* mask = exec_masks.back();
	Value* ret_value = builder().CreateICmpNE( mask, ext_->get_int(0) );
	return create_value(builtin_types::_boolean, value_array(1, ret_value), value_kinds::value, abis::llvm);
}

void cgs_simd::while_beg(){ enter_loop(); }
void cgs_simd::while_end(){ exit_loop(); }
void cgs_simd::while_cond_beg(){}
void cgs_simd::while_cond_end( multi_value const& cond )
{
	apply_loop_condition(cond);
}
void cgs_simd::while_body_beg() {}
void cgs_simd::while_body_end() { save_next_iteration_exec_mask(); }

void cgs_simd::enter_loop()
{
	mask_vars.push_back( restore( exec_masks.back() ) );
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

void cgs_simd::apply_loop_condition( multi_value const& cond )
{
	Value* exec_mask = load_loop_execution_mask();
	if( cond.abi() != abis::unknown ){
		value_array cond_exec_flags = cond.load();
		Value* cond_exec_mask = get_mask(cond_exec_flags);
		exec_mask = builder().CreateAnd(exec_mask, cond_exec_mask);
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
void cgs_simd::do_cond_end( multi_value const& cond ) {	
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
	return builder().CreateLoad( mask_vars.back() );
}

void cgs_simd::save_loop_execution_mask(Value* mask)
{
	builder().CreateStore( exec_masks.back(), mask_vars.back() );
}

multi_value cgs_simd::emit_and( multi_value const& lhs, multi_value const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cgs_simd::emit_or( multi_value const& lhs, multi_value const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

END_NS_SASL_CODEGEN();
