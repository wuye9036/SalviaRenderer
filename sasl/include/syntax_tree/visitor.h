#ifndef SASL_SYNTAX_TREE_VISITOR
#define SASL_SYNTAX_TREE_VISITOR

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

BEGIN_NS_SASL_SYNTAX_TREE()

struct unary_expression;
struct cast_expression;
struct expression_list;
struct cond_expression;
struct index_expression;
struct call_expression;
struct member_expression;
struct identifier;
struct constant_expression;
struct binary_expression;
struct constant;

struct initializer;
struct expression_initializer;
struct member_initializer;
struct declaration;
struct variable_declaration;
struct type_definition;
struct type_specifier;
struct buildin_type;
struct type_identifier;
struct qualified_type;
struct array_type;
struct struct_type;
struct parameter;
struct function_type;

struct statement;
struct declaration_statement;
struct if_statement;
struct while_statement;
struct dowhile_statement;
struct case_label;
struct switch_statement;
struct compound_statement;
struct expression_statement;
struct jump_statement;

class syntax_tree_visitor{
public:
	// expression
	virtual void visit( unary_expression& v ) = 0;
	virtual void visit( cast_expression& v) = 0;
	virtual void visit( binary_expression& v ) = 0;
	virtual void visit( expression_list& v ) = 0;
	virtual void visit( cond_expression& v ) = 0;
	virtual void visit( index_expression& v ) = 0;
	virtual void visit( call_expression& v ) = 0;
	virtual void visit( member_expression& v ) = 0;

	virtual void visit( constant_expression& v ) = 0;
	virtual void visit( constant& v ) = 0;
	virtual void visit( identifier& v ) = 0;

	// declaration & type specifier
	virtual void visit( initializer& v ) = 0;
	virtual void visit( expression_initializer& v ) = 0;
	virtual void visit( member_initializer& v ) = 0;
	virtual void visit( declaration& v ) = 0;
	virtual void visit( variable_declaration& v ) = 0;
	virtual void visit( type_definition& v ) = 0;
	virtual void visit( type_specifier& v ) = 0;
	virtual void visit( buildin_type& v ) = 0;
	virtual void visit( type_identifier& v ) = 0;
	virtual void visit( qualified_type& v ) = 0;
	virtual void visit( array_type& v ) = 0;
	virtual void visit( struct_type& v ) = 0;
	virtual void visit( parameter& v ) = 0;
	virtual void visit( function_type& v ) = 0;

	// statement
	virtual void visit( statement& v ) = 0;
	virtual void visit( declaration_statement& v ) = 0;
	virtual void visit( if_statement& v );
	virtual void visit( while_statement& v );
	virtual void visit( dowhile_statement& v );
	virtual void visit( case_label& v );
	virtual void visit( switch_statement& v );
	virtual void visit( compound_statement& v );
	virtual void visit( expression_statement& v );
	virtual void visit( jump_statement& v );
};

END_NS_SASL_SYNTAX_TREE()

#endif