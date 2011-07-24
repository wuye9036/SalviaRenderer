
#ifndef SASL_ENUMS_NODE_IDS_H
#define SASL_ENUMS_NODE_IDS_H

#include "../enums/enum_base.h" 

struct node_ids :
	public enum_base< node_ids, uint64_t >
	, public bitwise_op< node_ids >, public equal_op< node_ids >, public value_op< node_ids, uint64_t >
{
	friend struct enum_hasher;
private:
	node_ids( const storage_type& val, const std::string& name );
	node_ids( const storage_type& val ): base_type(val){}
public:
	static void force_initialize();
	
	node_ids( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type expression_statement;
	const static this_type member_expression;
	const static this_type unary_expression;
	const static this_type for_statement;
	const static this_type initializer;
	const static this_type function_type;
	const static this_type variable_declaration;
	const static this_type cond_expression;
	const static this_type case_label;
	const static this_type tynode;
	const static this_type compound_statement;
	const static this_type typedef_definition;
	const static this_type struct_type;
	const static this_type label;
	const static this_type while_statement;
	const static this_type program;
	const static this_type builtin_type;
	const static this_type switch_statement;
	const static this_type statement;
	const static this_type expression_initializer;
	const static this_type cast_expression;
	const static this_type if_statement;
	const static this_type parameter;
	const static this_type constant_expression;
	const static this_type node;
	const static this_type variable_expression;
	const static this_type dowhile_statement;
	const static this_type ident_label;
	const static this_type declaration;
	const static this_type array_type;
	const static this_type jump_statement;
	const static this_type alias_type;
	const static this_type binary_expression;
	const static this_type expression_list;
	const static this_type member_initializer;
	const static this_type declaration_statement;
	const static this_type index_expression;
	const static this_type declarator;
	const static this_type null_declaration;
	const static this_type identifier;
	const static this_type expression;
	const static this_type call_expression;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;

};

#endif
