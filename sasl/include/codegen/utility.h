#ifndef SASL_CODEGEN_UTILITY_H
#define SASL_CODEGEN_UTILITY_H

#include <sasl/include/codegen/forward.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/ADT/APInt.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/type_traits/is_signed.hpp>
#include <eflib/include/platform/boost_end.h>

namespace llvm
{
	class Function;
}

#define CGS_FUNCTION_SCOPE( fn ) \
	service()->push_fn( (fn) );	\
	cg_scope_guard<void> pop_fn_on_exit##__LINE__( bind( &cg_service::pop_fn, service() ) );

#define SEMANTIC_MODE_SCOPE( new_semantic_mode ) \
	scoped_value<bool> __sasl_semantic_mode_scope_##__LINE__( semantic_mode_, (new_semantic_mode) );
#define MSC_COMPATIBLE_SCOPE( new_msc_compatible ) \
	scoped_value<bool> __sasl_msc_compatible_scope_##__LINE__( msc_compatible_, (new_msc_compatible) );
#define CONTINUE_TO_SCOPE( new_continue_to ) \
	scoped_value<insert_point_t> __sasl_continue_to_scope_##__LINE__( continue_to_, (new_continue_to) );
#define BREAK_TO_SCOPE( new_break_to ) \
	scoped_value<insert_point_t> __sasl_break_to_scope_##__LINE__( break_to_, (new_break_to) );
#define TYPE_SCOPE( new_cg_type ) \
	scoped_value<cg_type*> __sasl_cg_type_scope_##__LINE__( current_cg_type_, (new_cg_type) );
#define STRUCT_SCOPE( new_struct ) \
	scoped_value<node_context*> __sasl_new_struct_scope_##__LINE__( parent_struct_, (new_struct) );
#define BLOCK_SCOPE( new_block ) \
	scoped_value<llvm::BasicBlock*> __sasl_block_scope_##__LINE__( block_, (new_block) );
#define SYMBOL_SCOPE( new_sym ) \
	scoped_value<symbol*> __sasl_sym_scope_##__LINE__( current_symbol_, (new_sym) );
#define VARIABLE_TO_INIT_SCOPE( var_to_init ) \
	scoped_value<node*> __sasl_var_to_init_scope_##__LINE__( variable_to_initialize_, (var_to_init) );

BEGIN_NS_SASL_CODEGEN();

template <typename T>
llvm::APInt apint( T v ){
	return llvm::APInt( sizeof(v) << 3, static_cast<uint64_t>(v), boost::is_signed<T>::value );
}

void		dbg_print_blocks( llvm::Function* fn );

END_NS_SASL_CODEGEN();

#endif
