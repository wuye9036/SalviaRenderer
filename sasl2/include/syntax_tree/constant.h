#ifndef SASL_SYNTAX_TREE_CONSTANT_H
#define SASL_SYNTAX_TREE_CONSTANT_H

#include "node.h"
#include "token.h"
#include "visitor.h"
#include <boost/variant.hpp>

struct constant: public node_impl<constant>{
	//literal_types lit_type;
	int val;

	constant()
		: node_impl<constant>( syntax_node_types::constant, token_attr::handle_t() ){}
	void update();
	void accept( syntax_tree_visitor* visitor ){
		visitor->visit( *this );
	}

protected:
	this_type& operator = (const this_type&);
	constant( const this_type& );
};

#endif //SASL_SYNTAX_TREE_CONSTANT_H