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

#define FUNCTION_SCOPE( fn ) \
	service()->push_fn( (fn) );	\
	scope_guard<void> pop_fn_on_exit##__LINE__( bind( &cg_service::pop_fn, service() ) );

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