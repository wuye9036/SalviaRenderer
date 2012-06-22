#ifndef SASL_CODE_GENERATOR_LLVM_UTILITY_H
#define SASL_CODE_GENERATOR_LLVM_UTILITY_H

#include <sasl/include/code_generator/forward.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/adt/APInt.h>
#include <eflib/include/platform/enable_warnings.h>

namespace llvm
{
	class Function;
}

#define CGS_FUNCTION_SCOPE( fn ) \
	service()->push_fn( (fn) );	\
	cg_scope_guard<void> pop_fn_on_exit##__LINE__( bind( &cg_service::pop_fn, service() ) );

#define SEMANTIC_MODE_SCOPE( new_semantic_mode ) \
	scope_guard<bool> __sasl_semantic_mode_scope_##__LINE__( semantic_mode_, (new_semantic_mode) );
#define MSC_COMPATIBLE_SCOPE( new_msc_compatible ) \
	scope_guard<bool> __sasl_msc_compatible_scope_##__LINE__( msc_compatible_, (new_msc_compatible) );
#define CONTINUE_TO_SCOPE( new_continue_to ) \
	scope_guard<insert_point_t> __sasl_continue_to_scope_##__LINE__( continue_to_, (new_continue_to) );
#define BREAK_TO_SCOPE( new_break_to ) \
	scope_guard<insert_point_t> __sasl_break_to_scope_##__LINE__( break_to_, (new_break_to) );
#define TYPE_SCOPE( new_cg_type ) \
	scope_guard<cg_type*> __sasl_cg_type_scope_##__LINE__( current_cg_type_, (new_cg_type) );
#define STRUCT_SCOPE( new_struct ) \
	scope_guard<node_context*> __sasl_new_struct_scope_##__LINE__( parent_struct_, (new_struct) );
#define BLOCK_SCOPE( new_block ) \
	scope_guard<llvm::BasicBlock*> __sasl_block_scope_##__LINE__( block_, (new_block) );
#define SYMBOL_SCOPE( new_sym ) \
	scope_guard<symbol*> __sasl_sym_scope_##__LINE__( current_symbol_, (new_sym) );
#define VARIABLE_TO_INIT_SCOPE( var_to_init ) \
	scope_guard<node*> __sasl_var_to_init_scope_##__LINE__( variable_to_initialize_, (var_to_init) );

BEGIN_NS_SASL_CODE_GENERATOR();

template <typename T>
llvm::APInt apint( T v ){
	return llvm::APInt( sizeof(v) << 3, static_cast<uint64_t>(v), boost::is_signed<T>::value );
}

void		mask_to_indexes( char indexes[4], uint32_t mask );
uint32_t	indexes_to_mask( char indexes[4] );
uint32_t	indexes_to_mask( char idx0, char idx1, char idx2, char idx3 );

uint32_t	indexes_length( char indexes[4] );
uint32_t	indexes_length( uint32_t mask );

void		dbg_print_blocks( llvm::Function* fn );

END_NS_SASL_CODE_GENERATOR();

#endif