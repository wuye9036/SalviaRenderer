#ifndef SASL_SYNTAX_TREE_PROGRAM_H
#define SASL_SYNTAX_TREE_PROGRAM_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <eflib/include/metaprog/enable_if.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <vector>

BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_visitor;
struct declaration;
struct token_t;

struct program: public node{

	SASL_SYNTAX_NODE_CREATORS();

	// help for creating program syntax tree
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::string name;
	std::vector< boost::shared_ptr<declaration> > decls;

protected:
	program(const std::string& name);
	program( boost::shared_ptr<token_t> const&, boost::shared_ptr<token_t> const& );
	program& operator = (const program&);
	program( const program& );
};

END_NS_SASL_SYNTAX_TREE();

#endif