#ifndef SASL_SYNTAX_TREE_IDENTIFIER_H
#define SASL_SYNTAX_TREE_IDENTIFIER_H

#include "node.h"
#include "token.h"
#include "visitor.h"
#include <string>

struct identifier: public node_impl<identifier>{
	std::string ident;

	identifier()
		: node_impl<identifier>( syntax_node_types::node, token_attr::handle_t() ){}
	void update();
	void accept( syntax_tree_visitor* visitor ){
		visitor->visit( *this );
	}

protected:
	this_type& operator = (const this_type&);
	identifier( const this_type& );
};


#endif