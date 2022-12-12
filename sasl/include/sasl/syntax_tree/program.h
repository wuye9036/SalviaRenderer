#ifndef SASL_SYNTAX_TREE_PROGRAM_H
#define SASL_SYNTAX_TREE_PROGRAM_H

#include <sasl/syntax_tree/syntax_tree_fwd.h>
#include <sasl/syntax_tree/node.h>
#include <eflib/utility/enable_if.h>

#include <vector>
#include <memory>

namespace sasl::syntax_tree() {

class syntax_tree_visitor;
struct declaration;
struct token_t;

struct program: public node{

	SASL_SYNTAX_NODE_CREATORS();

	// help for creating program syntax tree
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::string name;
	std::vector< std::shared_ptr<declaration> > decls;

protected:
	program(const std::string& name);
	program( std::shared_ptr<token_t> const&, std::shared_ptr<token_t> const& );
	program& operator = (const program&) = delete;
	program( const program& ) = delete;
};

}

#endif