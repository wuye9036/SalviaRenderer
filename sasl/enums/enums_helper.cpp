#include <sasl/enums/enums_helper.h>
#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/operators.h>
#include <eflib/include/disable_warnings.h>
#include <boost/assign/std/vector.hpp>
#include <eflib/include/enable_warnings.h>

bool sasl_ehelper::is_none( const buildin_type_code& btc ){
	return btc == buildin_type_code::none;
}

bool sasl_ehelper::is_void( const buildin_type_code& btc ){
	return btc == buildin_type_code::_void;
}

bool sasl_ehelper::is_integer( const buildin_type_code& btc )
{
	return ( btc & buildin_type_code::_generic_type_mask ) == buildin_type_code::_integer;
}

bool sasl_ehelper::is_real( const buildin_type_code& btc )
{
	return ( btc & buildin_type_code::_generic_type_mask ) == buildin_type_code::_real;
}

bool sasl_ehelper::is_signed( const buildin_type_code& btc )
{
	return ( btc & buildin_type_code::_sign_mask ) == buildin_type_code::_signed;
}

bool sasl_ehelper::is_unsigned( const buildin_type_code& btc )
{
	return ( btc & buildin_type_code::_sign_mask ) == buildin_type_code::_unsigned;
}

bool sasl_ehelper::is_scalar( const buildin_type_code& btc )
{
	bool scalar = ( ( btc & buildin_type_code::_dimension_mask ) == buildin_type_code::_scalar );
	return scalar && !is_void(btc) && !is_none(btc) ;
}

bool sasl_ehelper::is_vector( const buildin_type_code& btc )
{
	return ( btc & buildin_type_code::_dimension_mask ) == buildin_type_code::_vector;
}

bool sasl_ehelper::is_matrix( const buildin_type_code& btc )
{
	return ( btc & buildin_type_code::_dimension_mask ) == buildin_type_code::_matrix;
}

bool sasl_ehelper::is_storagable( const buildin_type_code& btc ){
	return 
		is_scalar(btc) || is_vector( btc ) || is_matrix( btc )
		;
}

buildin_type_code sasl_ehelper::scalar_of( const buildin_type_code& btc ){
	return ( btc & buildin_type_code::_scalar_type_mask );
}

buildin_type_code sasl_ehelper::vector_of( const buildin_type_code& btc, size_t len )
{
	if ( !is_scalar(btc) ){
		return buildin_type_code::none;
	}
	buildin_type_code ret = ( btc | buildin_type_code::_vector );
	ret.from_value(
		buildin_type_code::storage_type(
		ret.to_value() | ( len << ret._dim0_field_shift.to_value() )
		) 
		);
	return ret;
}

buildin_type_code sasl_ehelper::matrix_of( const buildin_type_code& btc, size_t len_0, size_t len_1 )
{
	if ( !is_scalar(btc) ){
		return buildin_type_code::none;
	}
	buildin_type_code ret( btc | buildin_type_code::_matrix );
	ret.from_value(
		buildin_type_code::storage_type(
		ret.to_value()
		| ( len_0 << ret._dim0_field_shift.to_value() )
		| ( len_1 << ret._dim1_field_shift.to_value() )
		)
		);

	return ret;
}

size_t sasl_ehelper::len_0( const buildin_type_code& btc )
{
	if( is_scalar(btc) ){
		return 1;
	}
	return (size_t)
		(
		(btc & buildin_type_code::_dim0_mask).to_value()
		>> buildin_type_code::_dim0_field_shift.to_value()
		);
}

size_t sasl_ehelper::len_1( const buildin_type_code& btc )
{
	if( is_scalar(btc) || is_vector(btc) ){
		return 1;
	}
	return (size_t)
		(
		(btc & buildin_type_code::_dim1_mask).to_value()
		>> buildin_type_code::_dim1_field_shift.to_value()
		);
}

size_t sasl_ehelper::storage_size( const buildin_type_code& btc ){
	if( is_none(btc) || is_void(btc) ){
		return 0;
	}
	size_t component_count = len_0(btc) * len_1(btc);
	size_t component_size = 0;
	buildin_type_code s_btc = scalar_of( btc );
	if( s_btc == buildin_type_code::_sint8 
		|| s_btc == buildin_type_code::_uint8 )
	{
		component_size = 1;
	} else if( s_btc == buildin_type_code::_sint16
		|| s_btc == buildin_type_code::_uint16 )
	{
		component_size = 2;
	} else if( s_btc == buildin_type_code::_sint32 
		|| s_btc == buildin_type_code::_uint32 
		|| s_btc == buildin_type_code::_float
		|| s_btc == buildin_type_code::_boolean)
	{
		component_size = 4;
	} else if( s_btc == buildin_type_code::_sint64
		|| s_btc == buildin_type_code::_uint64 
		|| s_btc == buildin_type_code::_double)
	{
		component_size = 8;
	}
	return component_size * component_count;
}

using namespace boost::assign;

boost::mutex sasl_ehelper::mtx_btlist_init;
std::vector<buildin_type_code> sasl_ehelper::btc_list;

const std::vector<buildin_type_code>& sasl_ehelper::list_of_buildin_type_codes(){
	boost::mutex::scoped_lock locker(mtx_btlist_init);
	if( btc_list.empty() ){
		// add scalars.
		btc_list +=	
			buildin_type_code::_sint8,
			buildin_type_code::_sint16,
			buildin_type_code::_sint32,
			buildin_type_code::_sint64,
			buildin_type_code::_uint8,
			buildin_type_code::_uint16,
			buildin_type_code::_uint32,
			buildin_type_code::_uint64,
			buildin_type_code::_boolean,
			buildin_type_code::_float,
			buildin_type_code::_double
			;

		// add vectors & matrixs
		size_t scalar_count = btc_list.size();
		for( size_t i_scalar = 0; i_scalar < scalar_count; ++i_scalar){
			for( int i = 1; i <= 4; ++i ){
				for( int j = 1; j <=4; ++j ){
					btc_list += matrix_of( btc_list[i_scalar], i, j );
				}
				btc_list += vector_of( btc_list[i_scalar], i );
			}
		}

		// add other types.
		btc_list +=
			buildin_type_code::none,
			buildin_type_code::_void
			;
	}
	return btc_list;
}

bool sasl_ehelper::is_arithmetic( const operators& op ){
	return 
		op == operators::add ||
		op == operators::sub ||
		op == operators::mul ||
		op == operators::div ||
		op == operators::mod
		;
}

bool sasl_ehelper::is_relationship( const operators& op ){
	return 
		op == operators::greater ||
		op == operators::greater_equal ||
		op == operators::equal ||
		op == operators::less ||
		op == operators::less_equal ||
		op == operators::not_equal
		;
}

bool sasl_ehelper::is_bit( const operators& op ){
	return
		op == operators::bit_and ||
		op == operators::bit_or ||
		op == operators::bit_xor
		;
}

bool sasl_ehelper::is_shift( const operators& op ){
	return
		op == operators::left_shift ||
		op == operators::right_shift
		;
}

bool sasl_ehelper::is_bool_arith( const operators& op ){
	return
		op == operators::logic_and ||
		op == operators::logic_or
		;
}

bool sasl_ehelper::is_prefix( const operators& op ){
	return
		op == operators::prefix_decr ||
		op == operators::prefix_incr
		;
}

bool sasl_ehelper::is_postfix( const operators& op ){
	return
		op == operators::postfix_decr ||
		op == operators::postfix_incr
		;
}

bool sasl_ehelper::is_unary_arith( const operators& op ){
	return
		op == operators::positive ||
		op == operators::negative
		;
}

bool sasl_ehelper::is_arith_assign( const operators& op ){
	return
		op == operators::add_assign ||
		op == operators::sub_assign ||
		op == operators::mul_assign ||
		op == operators::div_assign ||
		op == operators::mod_assign
		;
}

bool sasl_ehelper::is_bit_assign( const operators& op ){
	return 
		op == operators::bit_and_assign ||
		op == operators::bit_or_assign ||
		op == operators::bit_xor_assign
		;
}

bool sasl_ehelper::is_shift_assign( const operators& op ){
	return
		op == operators::lshift_assign ||
		op == operators::rshift_assign
		;
}

bool sasl_ehelper::is_assign( const operators& op ){
	return op == operators::assign;
}

boost::mutex sasl_ehelper::mtx_oplist_init;
std::vector<operators> sasl_ehelper::op_list;

const std::vector<operators>& sasl_ehelper::list_of_operators(){
	boost::mutex::scoped_lock locker(mtx_oplist_init);
	if ( op_list.empty() ){
		op_list +=
			operators::add, operators::add_assign,
			operators::assign,
			operators::bit_and, operators::bit_and_assign,
			operators::bit_not,
			operators::bit_or, operators::bit_or_assign,
			operators::bit_xor, operators::bit_xor_assign,
			operators::div, operators::div_assign,
			operators::equal, operators::greater, operators::greater_equal,
			operators::left_shift,
			operators::less, operators::less_equal,
			operators::logic_and, operators::logic_or, operators::logic_not,
			operators::lshift_assign,
			operators::mod, operators::mod_assign, operators::mul, operators::mul_assign,
			operators::negative,
			operators::none,
			operators::not_equal,
			operators::positive,
			operators::postfix_decr, operators::postfix_incr,
			operators::prefix_decr,	operators::prefix_incr,
			operators::right_shift, operators::rshift_assign,
			operators::sub, operators::sub_assign
			;
	}
	return op_list;
}

bool sasl_ehelper::is_standard( const buildin_type_code& btc ){
	if (btc == buildin_type_code::_sint32 ||
		btc == buildin_type_code::_sint64 ||
		btc == buildin_type_code::_uint64 ||
		btc == buildin_type_code::_float ||
		btc == buildin_type_code::_double) 
	{
			return true;
	}

	if ( is_vector(btc) || is_matrix(btc) ){
		return is_standard(scalar_of(btc));
	}

	return false;
}
