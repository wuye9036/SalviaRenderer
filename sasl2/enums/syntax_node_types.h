
#ifndef SASL_ENUMS_SYNTAX_NODE_TYPES_H
#define SASL_ENUMS_SYNTAX_NODE_TYPES_H

#include "../enums/enum_base.h" 

struct syntax_node_types :
	public enum_base< syntax_node_types, int >
	, equal_op< syntax_node_types >
{
	friend struct enum_hasher;
private:
	syntax_node_types( const storage_type& val ): base_type( val ){}
	
public:
	syntax_node_types( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type node;
	const static this_type constant;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
