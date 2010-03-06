#include "../../include/syntax_tree/constant.h"
#include <boost/lexical_cast.hpp>

using namespace boost;

void constant::update(){
	val = boost::lexical_cast<int>(tok->lit);
}