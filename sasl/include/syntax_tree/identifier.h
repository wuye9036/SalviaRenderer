#ifndef SASL_SYNTAX_TREE_IDENTIFIER_H
#define SASL_SYNTAX_TREE_IDENTIFIER_H

#include "node.h"
#include <sasl/include/common/token_attr.h>
#include <string>

struct identifier{
	std::string name;
	identifier(const std::string& name ): name(name){}
};

#endif