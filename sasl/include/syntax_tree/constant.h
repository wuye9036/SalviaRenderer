#ifndef SASL_SYNTAX_TREE_CONSTANT_H
#define SASL_SYNTAX_TREE_CONSTANT_H

#include "syntax_tree_fwd.h"
#include <sasl/enums/literal_constant_types.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SYNTAX_TREE();

struct constant{
	template <typename R> friend void create_node();
	template <typename R, typename P0> friend void create_node(P0);

	typedef constant this_type;

	literal_constant_types valtype;
	sasl::common::token_attr littok;

	bool is_unsigned() const;
	bool is_long() const;
	bool is_double() const;
	bool is_single() const;

	constant();
protected:
	this_type& operator = (const this_type&);
	constant( const this_type& );
};

END_NS_SASL_SYNTAX_TREE()

#endif //SASL_SYNTAX_TREE_CONSTANT_H