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
			src = builder().CreateSelect( mask, src, dest_value, "Merged" );
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	} else if ( kind == vkind_swizzle ){
		if( is_vector( lhs.parent()->hint()) ){
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
							Value* elem_exec_mask = builder().CreateExtractElement( exec_masks.back(), int_(i_elem) );
							Value* elem_val = builder().CreateSelect( elem_exec_mask, new_val, old_val );
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
	inst->setAlignment(4);
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

value_t cgs_simd::emit_cmp_lt( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_cmp_le( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_cmp_eq( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_cmp_ne( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_cmp_ge( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_cmp_gt( value_t const& lhs, value_t const& rhs )
{
	abis promoted_abi = promote_abi( lhs.abi(), rhs.abi() );

	builtin_types hint = lhs.hint();
	assert( hint == rhs.hint() );
	assert( hint != builtin_types::none );

	Value* lhs_v = lhs.load( promoted_abi );
	Value* rhs_v = rhs.load( promoted_abi );

	Value* ret_v = NULL;

	if( is_scalar(hint) || is_vector(hint) ){	
		if( is_integer(hint) ){
			if( is_signed(hint) ){
				ret_v = builder().CreateICmpSGT( lhs_v, rhs_v );
			} else {
				ret_v = builder().CreateICmpSGT( lhs_v, rhs_v );
			}
		} else if( is_real(hint) ){
			ret_v = builder().CreateFCmpUGT( lhs_v, rhs_v );
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return create_value( builtin_types::_boolean, ret_v, vkind_value, promoted_abi );
}

value_t cgs_simd::create_scalar( llvm::Value* v, value_tyinfo* tyi )
{
	Type* ty = tyi->ty( abi_vectorize );
	Value* vectorize_v = Constant::getNullValue( ty );

	for( size_t i_elem = 0; i_elem < llvm::cast<VectorType>(ty)->getNumElements(); ++i_elem ){
		vectorize_v = builder().CreateInsertElement( vectorize_v, v, int_( static_cast<int>(i_elem) ) );
	}

	return create_value( tyi, vectorize_v, vkind_value, abi_vectorize );
}

void cgs_simd::function_beg()
{
	exec_masks.push_back( all_one_mask() );
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
	cond_exec_mask = cond.load( abi_package );
}

void cgs_simd::then_beg()
{
	Value* then_mask =  builder().CreateAnd( cond_exec_mask, exec_masks.back(), "mask.then" );
	exec_masks.push_back( then_mask );
}

void cgs_simd::then_end()
{
	exec_masks.pop_back();
}

void cgs_simd::else_beg()
{
	Value* inv_cond_exec_mask = builder().CreateNot( cond_exec_mask, "Cond.Inv" );
	Value* else_mask =  builder().CreateAnd( cond_exec_mask, exec_masks.back(), "mask.else" );
	exec_masks.push_back( else_mask );
}

void cgs_simd::else_end()
{
	exec_masks.pop_back();
}

END_NS_SASL_CODE_GENERATOR();
