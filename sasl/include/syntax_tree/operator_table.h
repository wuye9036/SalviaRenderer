#ifndef SASL_SYNTAX_TREE_OPERATOR_TABLE_H
#define SASL_SYNTAX_TREE_OPERATOR_TABLE_H

#include "syntax_tree_fwd.h"
#include "node.h"
#include <sasl/include/common/token_attr.h>
#include <sasl/enums/operators.h>
#include <boost/tr1/unordered_map.hpp>

BEGIN_NS_SASL_SYNTAX_TREE()

class operator_table{
	std::tr1::unordered_map<std::string, operators> lit2op;
	std::tr1::unordered_map<operators, std::string> op2lit;

	operator_table();
	operator_table& add( const std::string& lit, operators op );

public:
	static operator_table& instance();
	operators find( const std::string& lit, bool is_unary = false, bool is_postfix = false) const;
	const std::string& find( operators op ) const;
};

END_NS_SASL_SYNTAX_TREE()

#endif //SASL_SYNTAX_TREE_OPERATOR_LITERAL_H