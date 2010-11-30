
#ifndef SASL_ENUMS_BUILDIN_TYPE_CODE_H
#define SASL_ENUMS_BUILDIN_TYPE_CODE_H

#include "../enums/enum_base.h" 

struct buildin_type_code :
	public enum_base< buildin_type_code, uint32_t >
	, public bitwise_op< buildin_type_code >, public equal_op< buildin_type_code >, public value_op< buildin_type_code, uint32_t >
{
	friend struct enum_hasher;
private:
	buildin_type_code( const storage_type& val ): base_type( val ){}
	
public:
	buildin_type_code( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type _unsigned;
	const static this_type _sint32;
	const static this_type _c_int;
	const static this_type _sint16;
	const static this_type _generic_type_field_shift;
	const static this_type _scalar_type_mask;
	const static this_type _sign_mask;
	const static this_type _dim1_mask;
	const static this_type _boolean;
	const static this_type _generic_type_mask;
	const static this_type _sint8;
	const static this_type _scalar;
	const static this_type _sign_field_shift;
	const static this_type _float;
	const static this_type _dim0_field_shift;
	const static this_type _void;
	const static this_type _uint16;
	const static this_type _dimension_mask;
	const static this_type _dim1_field_shift;
	const static this_type _dimension_field_shift;
	const static this_type _double;
	const static this_type _matrix;
	const static this_type _sint64;
	const static this_type _real;
	const static this_type _scalar_field_shift;
	const static this_type _uint8;
	const static this_type _signed;
	const static this_type _vector;
	const static this_type none;
	const static this_type _uint32;
	const static this_type _precision_field_shift;
	const static this_type _uint64;
	const static this_type _dim0_mask;
	const static this_type _integer;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;

};

const buildin_type_code buildin_type_code::_unsigned ( UINT32_C( 34603008 ) );
const buildin_type_code buildin_type_code::_sint32 ( UINT32_C( 35848192 ) );
const buildin_type_code buildin_type_code::_c_int ( UINT32_C( 35979264 ) );
const buildin_type_code buildin_type_code::_sint16 ( UINT32_C( 35782656 ) );
const buildin_type_code buildin_type_code::_generic_type_field_shift ( UINT32_C( 24 ) );
const buildin_type_code buildin_type_code::_scalar_type_mask ( UINT32_C( 268369920 ) );
const buildin_type_code buildin_type_code::_sign_mask ( UINT32_C( 267386880 ) );
const buildin_type_code buildin_type_code::_dim1_mask ( UINT32_C( 255 ) );
const buildin_type_code buildin_type_code::_boolean ( UINT32_C( 50331648 ) );
const buildin_type_code buildin_type_code::_generic_type_mask ( UINT32_C( 251658240 ) );
const buildin_type_code buildin_type_code::_sint8 ( UINT32_C( 35717120 ) );
const buildin_type_code buildin_type_code::_scalar ( UINT32_C( 0 ) );
const buildin_type_code buildin_type_code::_sign_field_shift ( UINT32_C( 20 ) );
const buildin_type_code buildin_type_code::_float ( UINT32_C( 16842752 ) );
const buildin_type_code buildin_type_code::_dim0_field_shift ( UINT32_C( 8 ) );
const buildin_type_code buildin_type_code::_void ( UINT32_C( 67108864 ) );
const buildin_type_code buildin_type_code::_uint16 ( UINT32_C( 34734080 ) );
const buildin_type_code buildin_type_code::_dimension_mask ( UINT32_C( 4026531840 ) );
const buildin_type_code buildin_type_code::_dim1_field_shift ( UINT32_C( 0 ) );
const buildin_type_code buildin_type_code::_dimension_field_shift ( UINT32_C( 28 ) );
const buildin_type_code buildin_type_code::_double ( UINT32_C( 16908288 ) );
const buildin_type_code buildin_type_code::_matrix ( UINT32_C( 536870912 ) );
const buildin_type_code buildin_type_code::_sint64 ( UINT32_C( 35913728 ) );
const buildin_type_code buildin_type_code::_real ( UINT32_C( 16777216 ) );
const buildin_type_code buildin_type_code::_scalar_field_shift ( UINT32_C( 16 ) );
const buildin_type_code buildin_type_code::_uint8 ( UINT32_C( 34668544 ) );
const buildin_type_code buildin_type_code::_signed ( UINT32_C( 35651584 ) );
const buildin_type_code buildin_type_code::_vector ( UINT32_C( 268435456 ) );
const buildin_type_code buildin_type_code::none ( UINT32_C( 0 ) );
const buildin_type_code buildin_type_code::_uint32 ( UINT32_C( 34799616 ) );
const buildin_type_code buildin_type_code::_precision_field_shift ( UINT32_C( 16 ) );
const buildin_type_code buildin_type_code::_uint64 ( UINT32_C( 34865152 ) );
const buildin_type_code buildin_type_code::_dim0_mask ( UINT32_C( 65280 ) );
const buildin_type_code buildin_type_code::_integer ( UINT32_C( 33554432 ) );


#endif
