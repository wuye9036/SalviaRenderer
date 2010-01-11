#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include "../../enums/syntax_node_types.h"
#include "token.h"
#include <boost/shared_ptr.hpp>

struct token_attr;
class syntax_tree_visitor;

struct node{
	syntax_node_types type;
	token_attr::handle_t tok;
	
	virtual void update() = 0;
	virtual void accept( syntax_tree_visitor* visitor ) = 0;
protected:
	node(syntax_node_types type, const token_attr::handle_t& tok)
		: type(type), tok(tok){
	}
	virtual ~node(){}
};

template <typename DerivedT>
struct node_impl: public node{
	typedef DerivedT this_type;
	typedef node_impl<DerivedT> base_type;
	typedef boost::shared_ptr<DerivedT> handle_t;

	node_impl(syntax_node_types type, token_attr::handle_t tok)
		: node(type, tok){}
};

#endif //SASL_SYNTAX_TREE_NODE_H