
#ifndef SASL_INTEGER_TYPE_SUFFIXES_H
#define SASL_INTEGER_TYPE_SUFFIXES_H

#include "../enums/enum_base.h" 

struct integer_type_suffixes :
	public enum_base< integer_type_suffixes, int >
	, equal_op< integer_type_suffixes >
{
	friend struct enum_hasher;
private:
	integer_type_suffixes( const storage_type& val ): base_type( val ){}
	
public:
	integer_type_suffixes( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type ul;
	const static this_type lu;
	const static this_type u;
	const static this_type l;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
