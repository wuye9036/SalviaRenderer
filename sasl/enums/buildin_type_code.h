
#ifndef SASL_ENUMS_BUILDIN_TYPE_CODE_H
#define SASL_ENUMS_BUILDIN_TYPE_CODE_H

#include "../enums/enum_base.h" 

struct buildin_type_code :
	public enum_base< buildin_type_code, int >
	, bitwise_op< buildin_type_code >, equal_op< buildin_type_code >
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

	const static this_type _scalar;
	const static this_type none;
	const static this_type _unsigned;
	const static this_type _sint32;
	const static this_type _sint16;
	const static this_type _float;
	const static this_type _uint8;
	const static this_type _matrix;
	const static this_type _sint64;
	const static this_type _real;
	const static this_type _uint64;
	const static this_type _uint16;
	const static this_type _double;
	const static this_type _integer;
	const static this_type _signed;
	const static this_type _vector;
	const static this_type _uint32;
	const static this_type _sint8;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
