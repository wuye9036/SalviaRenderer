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

cgs_simd::cgs_simd()
	: cg_service(PACKAGE_ELEMENT_COUNT)
{
}

void cgs_simd::store(multi_value& lhs, multi_value const& rhs)
{
	assert( lhs.value_count() == rhs.value_count() );

	value_array selected_value;
	value_array address(selected_value.size(), NULL);
	value_kinds::id kind = lhs.kind();
	Value* mask = exec_masks.back();

	if( kind == value_kinds::reference )
	{
		address = lhs.raw();
		value_array new_value = rhs.load( lhs.abi() );
		value_array old_value = ext_->load(address);
		selected_value = ext_->select(mask, new_value, old_value);
	}
	else if ( kind == value_kinds::elements )
	{
		if( is_vector( lhs.parent()->hint()) )
		{
			assert( lhs.parent()->storable() );
			
			char indexes[4];
			mask_to_indexes( indexes, lhs.masks() );
			uint32_t idx_len = indexes_length( indexes );

			if(lhs.abi() == abis::c)
			{
				value_array parent_address = lhs.parent()->load_ref();
				for( size_t i_write_idx = 0; i_write_idx < idx_len; ++i_write_idx )
				{
					value_array mem_ptr = ext_->struct_gep(parent_address, indexes[i_write_idx]);
					value_array old_elem = ext_->load(mem_ptr);
					multi_value new_element_val = emit_extract_val( rhs, static_cast<int>(i_write_idx) );
					value_array masked_new_elem = ext_->select(mask, new_element_val.load( lhs.abi() ), old_elem);
					ext_->store(masked_new_elem, mem_ptr);
				}
				return;
			}
			else if(lhs.abi() == abis::llvm)
			{
				multi_value old_parent = lhs.parent()->to_rvalue();
				multi_value selected_parent = old_parent;

				for( size_t i_write_idx = 0; i_write_idx < idx_len; ++i_write_idx )
				{
					multi_value new_elem_val = emit_extract_val( rhs, static_cast<int>(i_write_idx) );
					multi_value old_elem_val = emit_extract_val( old_parent, static_cast<int>(indexes[i_write_idx]) );

					value_array selected_new_elem = ext_->select( mask, new_elem_val.load( lhs.abi() ), old_elem_val.load() );
					multi_value selected_new_elem_val = create_value(
						new_elem_val.hint(),
						selected_new_elem,
						value_kinds::value,
						new_elem_val.abi()
						);
					
					selected_parent = emit_insert_val(selected_parent, indexes[i_write_idx], selected_new_elem_val);
				}
				lhs.parent()->store(selected_parent);
				return;
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			address = lhs.load_ref();
			value_array new_value = rhs.load( lhs.abi() );
			value_array old_value = ext_->load(address);
			selected_value = ext_->select(mask, new_value, old_value);
		}
	}

	ext_->store(selected_value, address);
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
	exec_masks.push_back( fn().partial_execution ? fn().execution_mask().load()[0] : all_one_mask() );
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

void cgs_simd::if_cond_beg()
{
	// Do nothing
}

void cgs_simd::if_cond_end( multi_value const& cond )
{
	cond_exec_masks.push_back( combine_flags( cond.load(abis::llvm) ) );
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
	return derivation( v, dd_horizontal );
}

multi_value cgs_simd::emit_ddy( multi_value const& v )
{
	return derivation( v, dd_vertical );
}

multi_value cgs_simd::derivation(multi_value const& v, derivation_directional dd)
{
	int const PACKAGE_LINES = PACKAGE_ELEMENT_COUNT / PACKAGE_LINE_ELEMENT_COUNT;

	builtin_types hint = v.hint();
	builtin_types scalar_hint = is_scalar(hint) ? hint : scalar_of(hint);

	assert( parallel_factor_ == 16 );

	value_array values = v.load();
	value_array diff_values(parallel_factor_, NULL);

	multi_value source0 = create_value(
		v.ty(), v.hint(), value_array(1, NULL), value_kinds::value, v.abi()
		);
	multi_value source1 = source0;

	for(size_t i = 0; i < 4; i+=2)
	{
		for(size_t j = 0; j < 4; ++j)
		{
			size_t value_index0(0);
			size_t value_index1(0);

			if(dd == dd_horizontal)
			{
				value_index0 = j * 4 + i;
				value_index1 = value_index0 + 1;
			}
			else
			{
				value_index0 = i * 4 + j;
				value_index1 = value_index0 + 4;
			}
			
			Value* source_vm_value[2] = {values[value_index0], values[value_index1] };
			source0.emplace( value_array(1, source_vm_value[0]), value_kinds::value, v.abi() );
			source1.emplace( value_array(1, source_vm_value[1]), value_kinds::value, v.abi() );

			multi_value diff = emit_sub(source1, source0);
			diff_values[value_index0]
			= diff_values[value_index1]
			= diff.load()[0];
		}
	}

	return create_value( v.ty(), v.hint(), diff_values, value_kinds::value, v.abi() );
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
	return ext_->get_int<uint32_t>(0);
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
		Value* cond_exec_mask = combine_flags(cond_exec_flags);
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

Value* cgs_simd::current_execution_mask() const
{
	return exec_masks.back();
}

END_NS_SASL_CODEGEN();
