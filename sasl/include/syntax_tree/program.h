#ifndef SASL_SYNTAX_TREE_PROGRAM_H
#define SASL_SYNTAX_TREE_PROGRAM_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <boost/shared_ptr.hpp>
#include <vector>

BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_visitor;
struct declaration_statement;

struct program: public node{
	program();
	void accept( syntax_tree_visitor* v );

	std::vector< boost::shared_ptr<declaration_statement> > decls;
};

END_NS_SASL_SYNTAX_TREE();

#endif