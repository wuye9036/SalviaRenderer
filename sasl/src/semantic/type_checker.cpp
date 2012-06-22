#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <cassert>

BEGIN_NS_SASL_SEMANTIC();
using ::sasl::syntax_tree::builtin_type;
using ::sasl::syntax_tree::tynode;
using ::sasl::syntax_tree::function_type;
using ::sasl::syntax_tree::variable_declaration;

using ::boost::shared_ptr;
using ::boost::shared_polymorphic_cast;

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
			shared_polymorphic_cast<builtin_type>(lhs),
			shared_polymorphic_cast<builtin_type>(rhs)
			);
	}
	assert(!"need to be implemented!");
	return false;
}

END_NS_SASL_SEMANTIC();
