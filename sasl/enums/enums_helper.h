#ifndef SASL_ENUMS_ENUMS_HELPER_H
#define SASL_ENUMS_ENUMS_HELPER_H

struct buildin_type_code;

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

	static buildin_type_code scalar_of( const buildin_type_code& /*btc*/ );
	static buildin_type_code vector_of( const buildin_type_code& /*btc*/, size_t len );
	static buildin_type_code matrix_of( const buildin_type_code& /*btc*/, size_t len_0, size_t len_1 );

	static size_t len_0( const buildin_type_code& /*btc*/);
	static size_t len_1( const buildin_type_code& /*btc*/);

	static size_t storage_size( const buildin_type_code& /*btc*/ );
};

#endif