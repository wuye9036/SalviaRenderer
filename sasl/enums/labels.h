
#ifndef SASL_LABELS_H
#define SASL_LABELS_H

#include "../enums/enum_base.h" 

struct labels :
	public enum_base< labels, int >
	, equal_op< labels >
{
	friend struct enum_hasher;
private:
	labels( const storage_type& val ): base_type( val ){}
	
public:
	labels( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type _identifier;
	const static this_type _default;
	const static this_type _case;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
