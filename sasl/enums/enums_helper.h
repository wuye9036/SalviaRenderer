#ifndef SASL_ENUMS_ENUMS_HELPER_H
#define SASL_ENUMS_ENUMS_HELPER_H

#include <eflib/include/platform/disable_warnings.h>
#include <boost/thread/mutex.hpp>
#include <eflib/include/platform/enable_warnings.h>
#include <string>
#include <vector>

struct buildin_type_code;
struct operators;

class sasl_ehelper{
public:
	// buildin type code
	static bool is_none( const buildin_type_code& /*btc*/ );
	static bool is_void( const buildin_type_code& /*btc*/ );
	static bool is_integer( const buildin_type_code& /*btc*/ );
	static bool is_real( const buildin_type_code& /*btc*/ );
	static bool is_signed( const buildin_type_code& /*btc*/ );
	static bool is_unsigned( const buildin_type_code& /*btc*/ );
	static bool is_scalar( const buildin_type_code& /*btc*/ );
	static bool is_vector( const buildin_type_code& /*btc*/ );
	static bool is_matrix( const buildin_type_code& /*btc*/ );

	static bool is_storagable( const buildin_type_code& /*btc*/ );
	static bool is_standard( const buildin_type_code& /*btc*/ );

	static buildin_type_code scalar_of( const buildin_type_code& /*btc*/ );
	static buildin_type_code vector_of( const buildin_type_code& /*btc*/, size_t len );
	static buildin_type_code matrix_of( const buildin_type_code& /*btc*/, size_t len_0, size_t len_1 );

	static size_t len_0( const buildin_type_code& /*btc*/);
	static size_t len_1( const buildin_type_code& /*btc*/);

	static size_t storage_size( const buildin_type_code& /*btc*/ );

	static const std::vector<buildin_type_code>& list_of_buildin_type_codes();

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

private:
	static std::vector<buildin_type_code> btc_list;
	static boost::mutex mtx_btlist_init;

	static std::vector<operators> op_list;
	static boost::mutex mtx_oplist_init;
};

#endif