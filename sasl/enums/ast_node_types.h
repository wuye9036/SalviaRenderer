
#ifndef SASL_AST_NODE_TYPES_H
#define SASL_AST_NODE_TYPES_H

#include "../enums/enum_base.h" 

struct ast_node_types :
	public enum_base< ast_node_types, int64_t >
	, bitwise_op< ast_node_types >, equal_op< ast_node_types >
{
	friend struct enum_hasher;
private:
	ast_node_types( const storage_type& val ): base_type( val ){}
	
public:
	ast_node_types( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type unknown;
	const static this_type expression_list;
	const static this_type type_qual;
	const static this_type expression;
	const static this_type array_type;
	const static this_type type;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
