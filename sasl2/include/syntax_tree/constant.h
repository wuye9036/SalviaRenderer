#ifndef SASL_SYNTAX_TREE_CONSTANT_H
#define SASL_SYNTAX_TREE_CONSTANT_H

#include "node.h"
#include "token.h"
#include <boost/variant.hpp>

struct constant: public node{
	//literal_types lit_type;
	int val;
	explicit constant( int val = 0);
	constant& operator = (const token_attr& token);
	constant& operator = ( const constant& rhs );
	constant( const constant& rhs );
	constant& operator = ( int val );
protected:
	//inherited
	constant* clone_impl() const;
	constant* deepcopy_impl() const;
};

#endif //SASL_SYNTAX_TREE_CONSTANT_H