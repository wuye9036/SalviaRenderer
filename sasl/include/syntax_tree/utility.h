#ifndef SASL_SYNTAX_TREE_UTILITY_H
#define SASL_SYNTAX_TREE_UTILITY_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/enums_helper.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <boost/assign/list_inserter.hpp>

BEGIN_NS_SASL_SYNTAX_TREE();

//////////////////////////////////////////////////////////////////////////
// Concept:
//	*ContainerT*
//		is a compatible with associated container in boost.assign.
//		The prototype is
//
//		class ContainerT<buildin_type_code, buildin_type>
//
//	*Pred* is a unary predicate function. The prototype of pred is
//
//		bool pred( const buildin_type_code& );
//
//////////////////////////////////////////////////////////////////////////
template<typename ContainerT, typename PredT>
void map_of_buildin_type( ContainerT& cont, const PredT& pred){
	cont.clear();
	typedef const vector<buildin_type_code>& btc_list_t;
	btc_list_t btclst( sasl_ehelper::list_of_buildin_type_codes() );
	for( btc_list_t::iterator it = btclst.begin(); it != btclst.end(); ++it ){
		if ( pred(*it) ){
			boost::shared_ptr<buildin_type> bt = create_node<buildin_type>( token_attr::null() );
			bt->value_typecode = *it;
			insert( cont )(*it, bt );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Concept:
//	*ContainerT*
//		is a compatible with linear container in boost.assign.
//		The prototype is
//
//		class ContainerT<buildin_type_code>
//
//	*Pred* is a unary predicate function. The prototype of pred is
//
//		bool pred( const buildin_type_code& );
//
//////////////////////////////////////////////////////////////////////////
template<typename ContainerT, typename PredT>
void list_of_buildin_type( ContainerT& cont, const PredT& pred ){
	cont.clear();
	typedef const vector<buildin_type_code>& btc_list_t;
	btc_list_t btclst( sasl_ehelper::list_of_buildin_type_codes() );
	for( btc_list_t::iterator it = btclst.begin(); it != btclst.end(); ++it ){
		if ( pred(*it) ){
			cont += *it;
		}
	}
}
END_NS_SASL_SYNTAX_TREE();

#endif