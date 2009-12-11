#include <boost/lexical_cast.hpp>
#include "../../include/syntax_tree/constant.h"

using namespace boost;

constant::constant( int val )
: val(val), node( syntax_node_types::constant, token_attr() ){
}

constant::constant( const constant& rhs )
: val(rhs.val), node( syntax_node_types::constant, rhs.tok ){
}

constant& constant::operator = ( const constant& rhs ){
	tok = rhs.tok;
	val = rhs.val;
	return *this;
}

constant& constant::operator = ( const token_attr& tok ){
	this->tok = tok;
	val = lexical_cast<int>( tok.lit );
	return *this;
}

constant* constant::clone_impl() const{
	return new constant( val );
}

constant* constant::deepcopy_impl() const{
	return new constant( val );
}