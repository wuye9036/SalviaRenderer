#ifndef SASL_SYNTAX_TREE_UTILITY_H
#define SASL_SYNTAX_TREE_UTILITY_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <any>
#include <vector>
#include <functional>

BEGIN_NS_SASL_SYNTAX_TREE();

//////////////////////////////////////////////////////////////////////////
// Concept:
//	*ContainerT*
//		is a compatible with associated container in boost.assign.
//		The prototype is
//
//		class ContainerT<builtin_types, builtin_type>
//
//	*Pred* is a unary predicate function. The prototype of pred is
//
//		bool pred( const builtin_types& );
//
//////////////////////////////////////////////////////////////////////////
template<typename ContainerT, typename PredT>
void map_of_builtin_type( ContainerT& cont, const PredT& pred){
	cont.clear();
	typedef std::vector<builtin_types> btc_list_t;
	btc_list_t const& btclst( sasl::utility::list_of_builtin_types() );
	for( btc_list_t::const_iterator it = btclst.begin(); it != btclst.end(); ++it ){
		if ( pred(*it) ){
			std::shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null(), token_t::null() );
			bt->tycode = *it;
            cont[*it] = bt;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Concept:
//	*ContainerT*
//		is a compatible with linear container in boost.assign.
//		The prototype is
//
//		class ContainerT<builtin_types>
//
//	*Pred* is a unary predicate function. The prototype of pred is
//
//		bool pred( const builtin_types& );
//
//////////////////////////////////////////////////////////////////////////
template<typename ContainerT, typename PredT>
void list_of_builtin_type( ContainerT& cont, const PredT& pred ){
	cont.clear();
	typedef std::vector<builtin_types> btc_list_t;
	btc_list_t const& btclst( sasl::utility::list_of_builtin_types() );
	for( btc_list_t::const_iterator it = btclst.begin(); it != btclst.end(); ++it ){
		if ( pred(*it) ){
			cont.push_back(*it);
		}
	}
}

void follow_up_traversal(
	std::shared_ptr<node> root,
	std::function<void( node&, ::std::any* )> on_visit
	);

#define SASL_SYNTAX_NODE_IS_A( node, node_type ) ( (node)->node_class() == BOOST_PP_CAT(node_ids::, node_type) )

// node creators
std::shared_ptr<node> duplicate( std::shared_ptr<node> src );
std::shared_ptr<node> deep_duplicate( std::shared_ptr<node> src );

std::shared_ptr<builtin_type> create_builtin_type( const builtin_types& btc );

END_NS_SASL_SYNTAX_TREE();

#endif
