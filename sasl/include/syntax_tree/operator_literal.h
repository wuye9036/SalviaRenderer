#ifndef SASL_SYNTAX_TREE_OPERATOR_LITERAL_H
#define SASL_SYNTAX_TREE_OPERATOR_LITERAL_H

#include "syntax_tree_fwd.h"
#include "node.h"
#include "token.h"
#include <sasl/enums/operators.h>

BEGIN_NS_SASL_SYNTAX_TREE()

class operator_table{
	std::tr1::unordered_map<std::string, operators> lit2op;
	std::tr1::unordered_map<operators, std::string> op2lit;

	operator_table();
	operator_table& add( const std::string& lit, operators op );

public:
	static operator_table& instance();
	operators find( const string& lit, bool is_unary = false, bool is_postfix = false) const;
	const string& find( operators op ) const;
};

END_NS_SASL_SYNTAX_TREE()

#endif //SASL_SYNTAX_TREE_OPERATOR_LITERAL_H