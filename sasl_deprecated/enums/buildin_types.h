
#ifndef SASL_BUILDIN_TYPES_H
#define SASL_BUILDIN_TYPES_H

#include "../enums/enum_base.h" 

struct buildin_types :
	public enum_base< buildin_types, int >
	, bitwise_op< buildin_types >, equal_op< buildin_types >, value_op< buildin_types, int >
{
	friend struct enum_hasher;
private:
	buildin_types( const storage_type& val ): base_type( val ){}
	
public:
	buildin_types( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type sasl_struct;
	const static this_type sasl_bool;
	const static this_type sasl_int64;
	const static this_type sasl_int32;
	const static this_type sasl_vector;
	const static this_type sasl_unknown;
	const static this_type sasl_uint32;
	const static this_type sasl_float;
	const static this_type sasl_array;
	const static this_type sasl_matrix;
	const static this_type sasl_uint64;
	const static this_type sasl_half;
	const static this_type sasl_int16;
	const static this_type sasl_uint16;
	const static this_type sasl_int8;
	const static this_type sasl_uint8;
	const static this_type sasl_double;
	const static this_type sasl_function;
	const static this_type sasl_void;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
