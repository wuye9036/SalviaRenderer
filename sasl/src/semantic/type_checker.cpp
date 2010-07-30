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

std::string mangle_function_name( boost::shared_ptr<function_type> v ){
	name_mangler nm;
	return nm.mangle( v );
}

boost::shared_ptr<type_specifier> actual_type( boost::shared_ptr<type_specifier> orgtype ){
	if ( orgtype->node_class() == syntax_node_types::buildin_type ){
		return orgtype;
	}
	return extract_semantic_info<type_semantic_info>(orgtype)->full_type();
}

bool is_equal( boost::shared_ptr<buildin_type> type0, boost::shared_ptr<buildin_type> type1 ){
	return type0->value_typecode == type1->value_typecode;
}

bool is_equal( boost::shared_ptr<type_specifier> type0, boost::shared_ptr<type_specifier> type1 ){
	if ( type0 == type1 ){ return true; }
	if ( type0 == NULL || type1 == NULL ){ return false; }
	boost::shared_ptr<type_specifier> ftype0 = actual_type( type0 );
	boost::shared_ptr<type_specifier> ftype1 = actual_type( type1 );

	if ( ftype0->node_class() != ftype1->node_class() ){ return false; }

	if ( ftype0->node_class() == syntax_node_types::buildin_type ){
		return is_equal( 
			boost::shared_polymorphic_cast<buildin_type>(ftype0),
			boost::shared_polymorphic_cast<buildin_type>(ftype1)
			);
	}
	assert( !"Not Implemented." );
}

bool is_equal( boost::shared_ptr<function_type> lhs, boost::shared_ptr<function_type> rhs ){
	if ( lhs == rhs ) return true;
	if ( lhs == NULL || rhs == NULL ) return false;
	if ( lhs->name->str != rhs->name->str ) return false;
	if ( lhs->params.size() != rhs->params.size() ){ return false; }
	for ( size_t i_param = 0; i_param < lhs->params.size(); ++i_param ){
		boost::shared_ptr<variable_declaration> lhs_param = 
			boost::shared_polymorphic_cast<variable_declaration>( lhs->params[i_param] );
		boost::shared_ptr<variable_declaration> rhs_param = 
			boost::shared_polymorphic_cast<variable_declaration>( rhs->params[i_param] );
		if ( !is_equal( lhs_param->type_info, rhs_param->type_info ) ){	return false; }
	}
	return false;
}

END_NS_SASL_SEMANTIC();