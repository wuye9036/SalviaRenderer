#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/enums/syntax_node_types.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace sasl{ 
	namespace common{ 
		struct token_attr; 
	}

	namespace semantic{
		class symbol;
	}
}
BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_visitor;

using sasl::common::token_attr;

struct node{
	boost::shared_ptr<class sasl::semantic::symbol> symbol();
	boost::shared_ptr<token_attr> token();
	syntax_node_types node_class();

	virtual void accept( syntax_tree_visitor* visitor ) = 0;

protected:
	node(syntax_node_types tid, boost::shared_ptr<token_attr> tok);

	syntax_node_types				type_id;
	boost::shared_ptr<token_attr>	tok;
	boost::weak_ptr<class sasl::semantic::symbol>	sym;

	virtual ~node();
};

END_NS_SASL_SYNTAX_TREE();

#endif //SASL_SYNTAX_TREE_NODE_H