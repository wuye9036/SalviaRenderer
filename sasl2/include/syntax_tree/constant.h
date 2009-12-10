#ifndef SASL_SYNTAX_TREE_CONSTANT_H
#define SASL_SYNTAX_TREE_CONSTANT_H

#include "node.h"
#include <boost/variant.hpp>

struct constant: public node{
	//literal_types lit_type;
	int val;
	explicit constant( int val );

protected:
	constant( const constant& rhs );
	constant& operator = ( const constant& rhs );
	constant& operator = ( int val );

	//inherited
	constant* clone_impl() const;
	constant* deepcopy_impl() const;
};

#endif //SASL_SYNTAX_TREE_CONSTANT_H