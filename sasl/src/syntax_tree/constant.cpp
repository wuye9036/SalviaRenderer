#include "sasl/include/syntax_tree/constant.h"
#include <boost/lexical_cast.hpp>

using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE();

constant::constant(): valtype(literal_constant_types::integer){
}

bool constant::is_double() const{
	return valtype == literal_constant_types::real;
}

bool constant::is_unsigned() const{
	return valtype == literal_constant_types::integer;
}

bool constant::is_long() const{
	return valtype == literal_constant_types::integer;
}

bool constant::is_single() const{
	return valtype == literal_constant_types::real;
}

END_NS_SASL_SYNTAX_TREE();