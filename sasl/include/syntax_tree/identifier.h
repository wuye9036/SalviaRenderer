#ifndef SASL_SYNTAX_TREE_IDENTIFIER_H
#define SASL_SYNTAX_TREE_IDENTIFIER_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <string>

namespace sasl{ namespace common{
	struct token_attr;
} }

BEGIN_NS_SASL_SYNTAX_TREE();

using sasl::common::token_attr;

struct identifier: public node{
	identifier( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );
	std::string name;
};

END_NS_SASL_SYNTAX_TREE();

#endif