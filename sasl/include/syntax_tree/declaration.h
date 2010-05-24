#ifndef SASL_SYNTAX_TREE_DECLARATION_H
#define SASL_SYNTAX_TREE_DECLARATION_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/syntax_node_types.h>
#include <sasl/enums/type_qualifiers.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace sasl{
	namespace common{
		struct token_attr;
	}
}

BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_visitor;
struct type_specifier;
struct statement;
struct expression;

using sasl::common::token_attr;

struct initializer: public node{
protected:
	initializer(syntax_node_types type_id, boost::shared_ptr<token_attr> tok);
	initializer& operator = ( const initializer& );
	initializer( const initializer& );
};

struct expression_initializer: public initializer{
	
	SASL_SYNTAX_NODE_CREATORS();

	void accept( syntax_tree_visitor* v );
	boost::shared_ptr< expression > init_expr;
private:
	expression_initializer( boost::shared_ptr<token_attr> tok );
	expression_initializer& operator = ( const expression_initializer& );
	expression_initializer( const expression_initializer& );
};

struct member_initializer: public initializer{	
	SASL_SYNTAX_NODE_CREATORS();

	void accept( syntax_tree_visitor* v );
	std::vector< boost::shared_ptr<initializer> > sub_inits;
private:
	member_initializer( boost::shared_ptr<token_attr> tok );
	member_initializer& operator = ( const member_initializer& );
	member_initializer( const member_initializer& );
};

struct declaration: public node{
protected:
	declaration(syntax_node_types type_id, boost::shared_ptr<token_attr> tok);
	declaration& operator = ( const declaration& );
	declaration( const declaration& );
};

struct variable_declaration : public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr<type_specifier>	type_info;
	boost::shared_ptr<token_attr>		name;
	boost::shared_ptr<initializer>		init;

protected:
	variable_declaration(boost::shared_ptr<token_attr> tok);
	variable_declaration& operator = ( const variable_declaration& );
	variable_declaration( const variable_declaration& );
};

struct type_definition: public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr< type_specifier > type_info;
	boost::shared_ptr<token_attr> ident;

protected:
	type_definition( boost::shared_ptr<token_attr> tok );
	type_definition& operator = ( const type_definition& );
	type_definition( const type_definition& );
};

struct type_specifier: public declaration{
	buildin_type_code value_typecode;
	bool is_buildin() const;
protected:
	type_specifier(syntax_node_types type_id, boost::shared_ptr<token_attr> tok);
};

struct buildin_type: public type_specifier{
	SASL_SYNTAX_NODE_CREATORS();
	void accept( syntax_tree_visitor* v );
	bool is_buildin() const;
protected:
	buildin_type( boost::shared_ptr<token_attr> tok );
	buildin_type& operator = ( const buildin_type& );
	buildin_type( const buildin_type& );
};

struct qualified_type: public type_specifier{
	SASL_SYNTAX_NODE_CREATORS();
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr< type_specifier > inner_type;
	type_qualifiers qual;

protected:
	qualified_type( boost::shared_ptr<token_attr> tok );
	qualified_type& operator = ( const qualified_type& );
	qualified_type( const qualified_type& );
};

struct array_type: public type_specifier{
	void accept( syntax_tree_visitor* v );
	std::vector< boost::shared_ptr<expression> > array_lens;
	boost::shared_ptr< type_specifier > elem_type;
protected:
	array_type( boost::shared_ptr<token_attr> tok );
	array_type& operator = ( const array_type& );
	array_type( const array_type& );
};

struct struct_type: public type_specifier{
	SASL_SYNTAX_NODE_CREATORS();
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr< token_attr > name;
	std::vector< boost::shared_ptr<declaration> > decls;

protected:
	struct_type( boost::shared_ptr<token_attr> tok );
	struct_type& operator = ( const struct_type& );
	struct_type( const struct_type& );
};

struct parameter: public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	void accept( syntax_tree_visitor* v );

	boost::shared_ptr<type_specifier> param_type;
	boost::shared_ptr<token_attr> ident;
	boost::shared_ptr<initializer> init;

protected:
	parameter( boost::shared_ptr<token_attr> tok );
	parameter& operator = ( const parameter& );
	parameter( const parameter& );
};

struct function_type: public type_specifier{
	SASL_SYNTAX_NODE_CREATORS();
	void accept( syntax_tree_visitor* v );

	boost::shared_ptr< token_attr > name;
	boost::shared_ptr< type_specifier > retval_type;
	std::vector< boost::shared_ptr<parameter> > params;
	std::vector< boost::shared_ptr<statement> > stmts;

	bool is_declaration;
protected:
	function_type( boost::shared_ptr<token_attr> tok );
	function_type& operator = ( const function_type& );
	function_type( const function_type& );
};

END_NS_SASL_SYNTAX_TREE();

#endif