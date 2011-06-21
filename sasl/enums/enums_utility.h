#ifndef SASL_ENUMS_ENUMS_HELPER_H
#define SASL_ENUMS_ENUMS_HELPER_H

#include <eflib/include/platform/disable_warnings.h>
#include <boost/thread/mutex.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <string>
#include <vector>

struct builtin_types;
struct operators;

namespace sasl{
	namespace utility{
	
		// builtin type code
		static bool is_none( const builtin_types& /*btc*/ );
		static bool is_void( const builtin_types& /*btc*/ );
		static bool is_integer( const builtin_types& /*btc*/ );
		static bool is_real( const builtin_types& /*btc*/ );
		static bool is_signed( const builtin_types& /*btc*/ );
		static bool is_unsigned( const builtin_types& /*btc*/ );
		static bool is_scalar( const builtin_types& /*btc*/ );
		static bool is_vector( const builtin_types& /*btc*/ );
		static bool is_matrix( const builtin_types& /*btc*/ );

		static bool is_storagable( const builtin_types& /*btc*/ );
		static bool is_standard( const builtin_types& /*btc*/ );

		static builtin_types scalar_of( const builtin_types& /*btc*/ );
		static builtin_types vector_of( const builtin_types& /*btc*/, size_t len );
		static builtin_types matrix_of( const builtin_types& /*btc*/, size_t len_0, size_t len_1 );

		static size_t len_0( const builtin_types& /*btc*/);
		static size_t len_1( const builtin_types& /*btc*/);

		static size_t storage_size( const builtin_types& /*btc*/ );

		static const std::vector<builtin_types>& list_of_builtin_type_codes();

		//////////////////////////////////////////////////////////////////////////
		// operators
		// types
		static bool is_arithmetic( const operators& );
		static bool is_relationship( const operators& );
		static bool is_bit( const operators& );
		static bool is_shift( const operators& );
		static bool is_bool_arith( const operators& );

		// operand count
		static bool is_prefix( const operators& );
		static bool is_postfix( const operators& );
		static bool is_unary_arith( const operators& );

		// is assign?
		static bool is_arith_assign( const operators& );
		static bool is_bit_assign( const operators& );
		static bool is_shift_assign( const operators& );
		static bool is_assign( const operators& );

		static const std::vector<operators>& list_of_operators();
	}
}

#endif