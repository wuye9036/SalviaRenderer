
#ifndef SASL_CONTROL_STATEMENTS_H
#define SASL_CONTROL_STATEMENTS_H

#include "../enums/enum_base.h" 

struct control_statements :
	public enum_base< control_statements, int >
	, equal_op< control_statements >
{
	friend struct enum_hasher;
private:
	control_statements( const storage_type& val ): base_type( val ){}
	
public:
	control_statements( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type _return;
	const static this_type _continue;
	const static this_type _break;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
