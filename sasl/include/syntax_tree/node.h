#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include "syntax_tree_fwd.h"
#include <sasl/include/common/token_attr.h>
#include <sasl/enums/syntax_node_types.h>
#include <boost/shared_ptr.hpp>

BEGIN_NS_SASL_SYNTAX_TREE()

struct token_attr;
class syntax_tree_visitor;

struct node{
	syntax_node_types				type_id;
	boost::shared_ptr<token_attr>	tok;

	virtual void accept( syntax_tree_visitor* visitor ) = 0;
protected:
	node(syntax_node_types tid, boost::shared_ptr<token_attr> tok);
	virtual ~node();
};

END_NS_SASL_SYNTAX_TREE()

#endif //SASL_SYNTAX_TREE_NODE_H