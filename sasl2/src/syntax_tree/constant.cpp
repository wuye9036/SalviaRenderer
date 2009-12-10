#include "../../include/syntax_tree/constant.h"

using namespace boost;

constant::constant( int val )
: val(val), node( syntax_node_types::constant ){
}

constant::constant( const constant& rhs )
: val(rhs.val), node( syntax_node_types::constant ){
}

constant& constant::operator = ( const constant& rhs ){
	val = rhs.val;
	return *this;
}

constant& constant::operator = ( int val ){
	this->val = val;
	return *this;
}

constant* constant::clone_impl() const{
	return new constant( val );
}

constant* constant::deepcopy_impl() const{
	return new constant( val );
}