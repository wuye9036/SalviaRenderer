#include <sasl/semantic/type_checker.h>
#include <sasl/semantic/symbol.h>
#include <sasl/syntax_tree/declaration.h>
#include <cassert>

namespace sasl::semantic() {
using ::sasl::syntax_tree::builtin_type;
using ::sasl::syntax_tree::tynode;
using ::sasl::syntax_tree::function_full_def;
using ::sasl::syntax_tree::variable_declaration;

using std::shared_ptr;
using ::std::dynamic_pointer_cast;

bool type_equal( shared_ptr<builtin_type> lhs, shared_ptr<builtin_type> rhs ){
	return lhs->tycode == rhs->tycode;
}

bool type_equal( shared_ptr<tynode> lhs, shared_ptr<tynode> rhs ){
	// if lhs or rhs is an alias of type, get its actual type for comparison.
	if(lhs->node_class() == node_ids::alias_type ){
		assert(!"need to be implemented!");
		return false;
		// return type_equal( actual_type(lhs), rhs );
	}
	if ( rhs->node_class() == node_ids::alias_type ){
		assert(!"need to be implemented!");
		return false;
		// return type_equal( lhs, actual_type( rhs ) );
	}
	if ( lhs->node_class() != rhs->node_class() ){
		return false;
	}
	if( lhs->node_class() == node_ids::builtin_type ){
		return type_equal(
			dynamic_pointer_cast<builtin_type>(lhs),
			dynamic_pointer_cast<builtin_type>(rhs)
			);
	}
	assert(!"need to be implemented!");
	return false;
}

}
