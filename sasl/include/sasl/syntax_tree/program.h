#pragma once

#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <sasl/common/token.h>
#include <sasl/syntax_tree/node.h>

#include <eflib/utility/enable_if.h>

#include <vector>
#include <memory>

namespace sasl::syntax_tree {

class syntax_tree_visitor;
struct declaration;

using token = sasl::common::token;

struct program: public node{
	std::string name;
	std::vector< std::shared_ptr<declaration> > decls;
};

}