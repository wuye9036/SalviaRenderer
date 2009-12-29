#ifndef SASL_SYNTAX_TREE_CONSTANT_H
#define SASL_SYNTAX_TREE_CONSTANT_H

#include "adapt_instrusive_struct_handle.h"
#include "node.h"
#include "token.h"
#include <boost/variant.hpp>

struct constant: public node_impl<constant>{
	//literal_types lit_type;
	int val;

	constant()
		: node_impl<constant>( syntax_node_types::constant, token_attr::handle_t() ){}
	void update();

protected:
	this_type& operator = (const this_type&);
	constant( const this_type& );
};

#endif //SASL_SYNTAX_TREE_CONSTANT_H