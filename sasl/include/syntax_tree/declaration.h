#ifndef SASL_SYNTAX_TREE_DECLARATION_H
#define SASL_SYNTAX_TREE_DECLARATION_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/identifier.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/syntax_node_types.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace sasl{
	namespace common{
		struct token_attr;
	}
}

BEGIN_NS_SASL_SYNTAX_TREE()

struct type_specifier;
struct statement;
struct expression;

using sasl::common::token_attr;

struct initializer: public node{
	initializer(syntax_node_types type_id, boost::shared_ptr<token_attr> tok);
};

struct expression_initializer: public initializer{
	expression_initializer( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr< expression > init_expr;
};

struct member_initializer: public initializer{
	member_initializer( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );
	std::vector< boost::shared_ptr<initializer> > sub_inits;
};

struct declaration: public node{
	declaration(syntax_node_types type_id, boost::shared_ptr<token_attr> tok);
	void accept( syntax_tree_visitor* v );
	boost::weak_ptr<symbol> sym;
};

struct variable_declaration : public declaration{
	variable_declaration(boost::shared_ptr<token_attr> tok);
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr<type_specifier>	type_info;
	boost::shared_ptr<token_attr>		name;
	boost::shared_ptr<initializer>		init;
};

struct type_definition: public declaration{
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr< type_speficier > type_info;
	token_attr ident;
};

struct type_specifier: public declaration{
	type_specifier(syntax_node_types type_id, boost::shared_ptr<token_attr> tok);
	buildin_type_code type_id_of_value;
	virtual bool is_buildin() const = 0;
};

struct buildin_type: public type_specifier{
	buildin_type( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );
	bool is_buildin() const;
};

struct type_identifier: public type_specifier{
	type_identifier( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );

	boost::shared_ptr< type_speficier > inner_type;
	std::string name;
};

struct qualified_type: public type_specifier{
	qualified_type( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr< type_speficier > inner_type;
	type_qualifiers qual;
};

struct array_type: public type_specifier{
	array_type( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );
	std::vector< boost::shared_ptr<expression> > array_lens;
	boost::shared_ptr< type_specifier > elem_type;
};

struct struct_type: public type_specifier{
	struct_type( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );
	boost::shared_ptr< token_attr > name;
	std::vector< boost::shared_ptr<declaration> > decls;
};

struct parameter{
	boost::shared_ptr<type_specifier> param_type;
	boost::shared_ptr<identifier> ident;
	boost::shared_ptr<initializer> init;
};

struct function_type: public type_specifier{
	function_type( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );

	boost::shared_ptr< token_attr > name;
	std::vector< boost::shared_ptr<parameter> > params;
	std::vector< boost::shared_ptr<statement> > stmts;
};

END_NS_SASL_SYNTAX_TREE()

#endif