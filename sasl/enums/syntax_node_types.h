
#ifndef SASL_ENUMS_SYNTAX_NODE_TYPES_H
#define SASL_ENUMS_SYNTAX_NODE_TYPES_H

#include "../enums/enum_base.h" 

struct syntax_node_types :
	public enum_base< syntax_node_types, uint64_t >
	, public bitwise_op< syntax_node_types >, public equal_op< syntax_node_types >, public value_op< syntax_node_types, uint64_t >
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

	const static this_type expression_statement;
	const static this_type member_expression;
	const static this_type unary_expression;
	const static this_type for_statement;
	const static this_type initializer;
	const static this_type function_type;
	const static this_type variable_declaration;
	const static this_type cond_expression;
	const static this_type case_label;
	const static this_type type_specifier;
	const static this_type compound_statement;
	const static this_type typedef_definition;
	const static this_type struct_type;
	const static this_type label;
	const static this_type while_statement;
	const static this_type program;
	const static this_type switch_statement;
	const static this_type statement;
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
	const static this_type buildin_type;
	const static this_type binary_expression;
	const static this_type expression_list;
	const static this_type member_initializer;
	const static this_type declaration_statement;
	const static this_type index_expression;
	const static this_type expression_initializer;
	const static this_type null_declaration;
	const static this_type identifier;
	const static this_type expression;
	const static this_type call_expression;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;

};

const syntax_node_types syntax_node_types::expression_statement ( uint64_t( 844424930131975 ) );
const syntax_node_types syntax_node_types::member_expression ( uint64_t( 562949953421322 ) );
const syntax_node_types syntax_node_types::unary_expression ( uint64_t( 562949953421315 ) );
const syntax_node_types syntax_node_types::for_statement ( uint64_t( 844424930131977 ) );
const syntax_node_types syntax_node_types::initializer ( uint64_t( 1125899906842624 ) );
const syntax_node_types syntax_node_types::function_type ( uint64_t( 281479271677956 ) );
const syntax_node_types syntax_node_types::variable_declaration ( uint64_t( 281474976710657 ) );
const syntax_node_types syntax_node_types::cond_expression ( uint64_t( 562949953421319 ) );
const syntax_node_types syntax_node_types::case_label ( uint64_t( 1970324836974594 ) );
const syntax_node_types syntax_node_types::type_specifier ( uint64_t( 281479271677952 ) );
const syntax_node_types syntax_node_types::compound_statement ( uint64_t( 844424930131974 ) );
const syntax_node_types syntax_node_types::typedef_definition ( uint64_t( 281474976710658 ) );
const syntax_node_types syntax_node_types::struct_type ( uint64_t( 281479271677955 ) );
const syntax_node_types syntax_node_types::label ( uint64_t( 1970324836974592 ) );
const syntax_node_types syntax_node_types::while_statement ( uint64_t( 844424930131971 ) );
const syntax_node_types syntax_node_types::program ( uint64_t( 1688849860263936 ) );
const syntax_node_types syntax_node_types::switch_statement ( uint64_t( 844424930131973 ) );
const syntax_node_types syntax_node_types::statement ( uint64_t( 844424930131968 ) );
const syntax_node_types syntax_node_types::cast_expression ( uint64_t( 562949953421316 ) );
const syntax_node_types syntax_node_types::if_statement ( uint64_t( 844424930131970 ) );
const syntax_node_types syntax_node_types::parameter ( uint64_t( 281474976710659 ) );
const syntax_node_types syntax_node_types::constant_expression ( uint64_t( 562949953421313 ) );
const syntax_node_types syntax_node_types::node ( uint64_t( 0 ) );
const syntax_node_types syntax_node_types::variable_expression ( uint64_t( 562949953421314 ) );
const syntax_node_types syntax_node_types::dowhile_statement ( uint64_t( 844424930131972 ) );
const syntax_node_types syntax_node_types::ident_label ( uint64_t( 1970324836974593 ) );
const syntax_node_types syntax_node_types::declaration ( uint64_t( 281474976710656 ) );
const syntax_node_types syntax_node_types::array_type ( uint64_t( 281479271677954 ) );
const syntax_node_types syntax_node_types::jump_statement ( uint64_t( 844424930131976 ) );
const syntax_node_types syntax_node_types::alias_type ( uint64_t( 281479271677957 ) );
const syntax_node_types syntax_node_types::buildin_type ( uint64_t( 281479271677953 ) );
const syntax_node_types syntax_node_types::binary_expression ( uint64_t( 562949953421317 ) );
const syntax_node_types syntax_node_types::expression_list ( uint64_t( 562949953421318 ) );
const syntax_node_types syntax_node_types::member_initializer ( uint64_t( 1125899906842626 ) );
const syntax_node_types syntax_node_types::declaration_statement ( uint64_t( 844424930131969 ) );
const syntax_node_types syntax_node_types::index_expression ( uint64_t( 562949953421320 ) );
const syntax_node_types syntax_node_types::expression_initializer ( uint64_t( 1125899906842625 ) );
const syntax_node_types syntax_node_types::null_declaration ( uint64_t( 281474976710660 ) );
const syntax_node_types syntax_node_types::identifier ( uint64_t( 1407374883553280 ) );
const syntax_node_types syntax_node_types::expression ( uint64_t( 562949953421312 ) );
const syntax_node_types syntax_node_types::call_expression ( uint64_t( 562949953421321 ) );


#endif
