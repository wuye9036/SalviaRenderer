#include <sasl/enums/enums_helper.h>
#include <sasl/enums/buildin_type_code.h>

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
