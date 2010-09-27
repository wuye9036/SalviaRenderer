#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <cassert>

BEGIN_NS_SASL_SEMANTIC();
using ::sasl::syntax_tree::buildin_type;
using ::sasl::syntax_tree::type_specifier;
using ::sasl::syntax_tree::function_type;
using ::sasl::syntax_tree::variable_declaration;

bool type_equal( boost::shared_ptr<buildin_type> lhs, boost::shared_ptr<buildin_type> rhs ){
	return lhs->value_typecode == rhs->value_typecode;
}

bool type_equal( boost::shared_ptr<::sasl::syntax_tree::type_specifier> lhs, boost::shared_ptr<::sasl::syntax_tree::type_specifier> rhs ){
	// if lhs or rhs is an alias of type, get its actual type for comparison.
	if(lhs->node_class() == syntax_node_types::alias_type ){
		assert(!"need to be implemented!");
		return false;
		// return type_equal( actual_type(lhs), rhs );
	}
	if ( rhs->node_class() == syntax_node_types::alias_type ){
		assert(!"need to be implemented!");
		return false;
		// return type_equal( lhs, actual_type( rhs ) );
	}
	if ( lhs->node_class() != rhs->node_class() ){
		return false;
	}
	if( lhs->node_class() == syntax_node_types::buildin_type ){
		return type_equal(
			boost::shared_polymorphic_cast<buildin_type>(lhs),
			boost::shared_polymorphic_cast<buildin_type>(rhs)
			);
	}
	assert(!"need to be implemented!");
	return false;
}

END_NS_SASL_SEMANTIC();