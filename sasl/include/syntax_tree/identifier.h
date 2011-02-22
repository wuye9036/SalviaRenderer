#ifndef SASL_SYNTAX_TREE_IDENTIFIER_H
#define SASL_SYNTAX_TREE_IDENTIFIER_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <string>

namespace sasl{ namespace common{
	struct token_t;
} }

BEGIN_NS_SASL_SYNTAX_TREE();

using sasl::common::token_t;

struct identifier: public node{
	identifier( boost::shared_ptr<token_t> tok );
	std::string name;
};

END_NS_SASL_SYNTAX_TREE();

#endif