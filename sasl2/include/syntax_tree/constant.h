#ifndef SASL_SYNTAX_TREE_CONSTANT_H
#define SASL_SYNTAX_TREE_CONSTANT_H

#include "adapt_instrusive_struct_handle.h"
#include "node.h"
#include "token.h"
#include <boost/variant.hpp>

struct constant: public terminal<constant>{
	//literal_types lit_type;
	int val;

	constant()
		: terminal<constant>( syntax_node_types::constant, token_attr::handle_t() ){}
	void update();

protected:
	this_type& operator = (const this_type&);
	constant( const this_type& );
};

//SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE( 
//									constant::handle_t, 
//									( token_attr::handle_t, tok )
//									);

#endif //SASL_SYNTAX_TREE_CONSTANT_H