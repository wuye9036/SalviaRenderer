
#ifndef SASL_ENUMS_TYPE_TYPES_H
#define SASL_ENUMS_TYPE_TYPES_H

#include "../enums/enum_base.h" 

struct type_types :
	public enum_base< type_types, int >
	, equal_op< type_types >, value_op< type_types, int >
{
	friend struct enum_hasher;
private:
	type_types( const storage_type& val ): base_type( val ){}
	
public:
	type_types( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type alias;
	const static this_type none;
	const static this_type buildin;
	const static this_type composited;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
