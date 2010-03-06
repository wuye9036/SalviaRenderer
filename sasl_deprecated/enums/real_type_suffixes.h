
#ifndef SASL_REAL_TYPE_SUFFIXES_H
#define SASL_REAL_TYPE_SUFFIXES_H

#include "../enums/enum_base.h" 

struct real_type_suffixes :
	public enum_base< real_type_suffixes, int >
	, equal_op< real_type_suffixes >
{
	friend struct enum_hasher;
private:
	real_type_suffixes( const storage_type& val ): base_type( val ){}
	
public:
	real_type_suffixes( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type e;
	const static this_type f;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
