#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include "../../enums/syntax_node_types.h"
#include "token.h"
#include <boost/utility.hpp>

struct node{
	token_attr tok;
	syntax_node_types type;

	template <typename NodeT> NodeT* clone() const{
		return static_cast<NodeT*>( clone_impl() );
	}

	template <typename NodeT> NodeT* deepcopy() const{
		return static_cast<NodeT*>( deepcopy_impl() );
	}

protected:
	node(syntax_node_types type, const token_attr& tok): type(type), tok(tok){
	}
	node( const node& rhs ): type(rhs.type), tok(tok){
	}
	node& operator = ( const node& rhs ){
		type = rhs.type;
		tok = rhs.tok;
	}
	
	virtual node* clone_impl() const = 0;
	virtual node* deepcopy_impl() const = 0;
};

#endif //SASL_SYNTAX_TREE_NODE_H