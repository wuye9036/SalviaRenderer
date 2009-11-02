
#ifndef SASL_TYPE_QUALIFIERS_H
#define SASL_TYPE_QUALIFIERS_H

#include "../enums/enum_base.h" 

struct type_qualifiers :
	public enum_base< type_qualifiers, int >
	, equal_op< type_qualifiers >
{
	friend struct enum_hasher;
private:
	type_qualifiers( const storage_type& val ): base_type( val ){}
	
public:
	type_qualifiers( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type _uniform;
	const static this_type _const;
	const static this_type _volatile;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
