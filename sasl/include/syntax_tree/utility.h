#ifndef SASL_SYNTAX_TREE_UTILITY_H
#define SASL_SYNTAX_TREE_UTILITY_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/enums_helper.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

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
	typedef std::vector<buildin_type_code> btc_list_t;
	const btc_list_t& btclst( sasl_ehelper::list_of_buildin_type_codes() );
	for( btc_list_t::const_iterator it = btclst.begin(); it != btclst.end(); ++it ){
		if ( pred(*it) ){
			boost::shared_ptr<buildin_type> bt = create_node<buildin_type>( token_t::null() );
			bt->value_typecode = *it;
			boost::assign::insert( cont )(*it, bt );
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
	typedef std::vector<buildin_type_code> btc_list_t;
	const btc_list_t& btclst( sasl_ehelper::list_of_buildin_type_codes() );
	for( btc_list_t::const_iterator it = btclst.begin(); it != btclst.end(); ++it ){
		if ( pred(*it) ){
			cont += *it;
		}
	}
}

void follow_up_traversal(
	boost::shared_ptr<node> root,
	boost::function<void( node&, ::boost::any* )> on_visit
	);

#define SASL_SYNTAX_NODE_IS_A( node, node_type ) ( (node)->node_class() == BOOST_PP_CAT(syntax_node_types::, node_type) )

// node creators
boost::shared_ptr<node> duplicate( ::boost::shared_ptr<node> src );
boost::shared_ptr<node> deep_duplicate( ::boost::shared_ptr<node> src );

boost::shared_ptr<buildin_type> create_buildin_type( const buildin_type_code& btc );

END_NS_SASL_SYNTAX_TREE();

#endif
