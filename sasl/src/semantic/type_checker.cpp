#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <cassert>

BEGIN_NS_SASL_SEMANTIC();
using ::sasl::syntax_tree::buildin_type;

bool is_equal( boost::shared_ptr<buildin_type> type0, boost::shared_ptr<buildin_type> type1 ){
	return type0->value_typecode == type1->value_typecode;
}

bool is_equal( boost::shared_ptr<type_specifier> type0, boost::shared_ptr<type_specifier> type1 ){
	if ( type0 == type1 ){ return true; }
	if ( type0 == NULL || type1 == NULL ){ return false; }
	if ( type0->node_class() != type1->node_class() ){ return false; }

	if ( type0->node_class() == syntax_node_types::buildin_type ){
		return is_equal( 
			boost::shared_polymorphic_cast<buildin_type>(type0),
			boost::shared_polymorphic_cast<buildin_type>(type1)
			);
	}
	assert( !"Not Implemented." );
}

END_NS_SASL_SEMANTIC();