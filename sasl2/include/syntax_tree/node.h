#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include "../../enums/syntax_node_types.h"
#include <boost/utility.hpp>

struct node{
	// token_location loc;
	syntax_node_types type;

	template <typename NodeT> NodeT* clone() const{
		return static_cast<NodeT*>( clone_impl() );
	}

	template <typename NodeT> NodeT* deepcopy() const{
		return static_cast<NodeT*>( deepcopy_impl() );
	}

protected:
	explicit node(syntax_node_types type): type(type){
	}
	node( const node& rhs ): type(rhs.type){
	}
	node& operator = ( const node& rhs ){
		type = rhs.type;
	}
	virtual node* clone_impl() const = 0;
	virtual node* deepcopy_impl() const = 0;
};

#endif //SASL_SYNTAX_TREE_NODE_H