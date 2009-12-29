#ifndef SASL_SYNTAX_TREE_OPERATOR_LITERAL_H
#define SASL_SYNTAX_TREE_OPERATOR_LITERAL_H

#include "node.h"
#include "token.h"
#include "adapt_instrusive_struct_handle.h"
#include "../../enums/operators.h"

struct operator_literal: public node_impl<operator_literal>{
	
	operators op;

	operator_literal( )
		: node_impl< operator_literal >( syntax_node_types::node, token_attr::handle_t() ), op( operators::none ){}

	void update();

protected:
	this_type& operator = (const this_type&);
	operator_literal( const this_type& );
};

#endif //SASL_SYNTAX_TREE_OPERATOR_LITERAL_H