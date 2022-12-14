#ifndef SASL_SYNTAX_TREE_IDENTIFIER_H
#define SASL_SYNTAX_TREE_IDENTIFIER_H

#include <sasl/syntax_tree/syntax_tree_fwd.h>
#include <sasl/syntax_tree/node.h>
#include <string>

namespace sasl{ namespace common{
	struct token_t;
} }

namespace sasl::syntax_tree {

using sasl::common::token_t;

struct identifier: public node{
	identifier( std::shared_ptr<token_t> const& tok );
	std::string name;
};

}

#endif