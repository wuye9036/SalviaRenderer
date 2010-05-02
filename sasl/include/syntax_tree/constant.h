#ifndef SASL_SYNTAX_TREE_CONSTANT_H
#define SASL_SYNTAX_TREE_CONSTANT_H

#include "syntax_tree_fwd.h"
#include <sasl/enums/literal_constant_types.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SYNTAX_TREE()

struct constant{
	typedef constant this_type;

	literal_constant_types valtype;
	sasl::common::token_attr lit;

	bool is_unsigned();
	bool is_long();
	bool is_double();
	bool is_single();

	constant();
protected:
	this_type& operator = (const this_type&);
	constant( const this_type& );
};

END_NS_SASL_SYNTAX_TREE()

#endif //SASL_SYNTAX_TREE_CONSTANT_H