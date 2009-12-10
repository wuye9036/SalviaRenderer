#ifndef SASL_SYNTAX_TREE_TOKEN_H
#define SASL_SYNTAX_TREE_TOKEN_H

#include <string>
#include <boost/mpl/vector.hpp>

struct token_location{
	std::string file_name;
	std::size_t line;
	std::size_t column;
};

typedef boost::mpl::vector3<token_types, std::string, token_location> sasl_token; 

#endif