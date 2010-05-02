#ifndef SASL_SYNTAX_TREE_EXPRESSION_H
#define SASL_SYNTAX_TREE_EXPRESSION_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/operators.h>
#include <boost/smart_ptr.hpp>
#include <string>

namespace sasl { 
	namespace common {
		struct token_attr;
	}
}

BEGIN_NS_SASL_SYNTAX_TREE();

using sasl::common::token_attr;
class syntax_tree_visitor;

struct expression: public node{
	expression( syntax_node_types nodetype, boost::shared_ptr<token_attr> tok);
};

struct constant_expression: public expression{
	constant_expression( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor );
	boost::shared_ptr<constant> value;
};

struct unary_expression: public expression{
	unary_expression( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor );

	boost::shared_ptr<expression> expr;
	operators op;
};

struct cast_expression: public expression{
	cast_expression( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor);

	boost::shared_ptr<identifier> casted_type_name;
	boost::shared_ptr<expression> expr;
};

struct binary_expression: public expression {
	binary_expression( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor );

	operators op;
	boost::shared_ptr<expression> left_expr;
	boost::shared_ptr<expression> right_expr;
};

struct expression_list: public expression{
	expression_list( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor);

	std::vector< boost::shared_ptr<expression> > exprs;
};

struct cond_expression: public expression{
	cond_expression( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor );

	boost::shared_ptr<expression> cond_expr;
	boost::shared_ptr<expression> yes_expr;
	boost::shared_ptr<expression> no_expr;
};

struct index_expression: public expression{
	index_expression( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor );

	boost::shared_ptr<expression> expr;
	boost::shared_ptr<expression> index_expr;
};

struct call_expression: public expression{
	call_expression( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor );

	boost::shared_ptr<expression> expr;
	std::vector<boost::shared_ptr<expression> > args;
};

struct member_expression: public expression{
	member_expression( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* visitor );
	boost::shared_ptr<expression> expr;
	boost::shared_ptr<identifier> member_ident;
};

END_NS_SASL_SYNTAX_TREE();

#endif //SASL_SYNTAX_TREE_EXPRESSION_H