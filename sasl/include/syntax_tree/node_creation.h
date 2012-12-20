#ifndef SASL_SYNTAX_TREE_NODE_CREATION_H
#define SASL_SYNTAX_TREE_NODE_CREATION_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/tr1/type_traits.hpp>

BEGIN_NS_SASL_SYNTAX_TREE();

struct node;

template <typename NodeT> boost::shared_ptr<NodeT> create_node(){
	BOOST_STATIC_ASSERT( ( std::tr1::is_base_of<node, NodeT>::value ) );
	boost::shared_ptr<NodeT> ret( new NodeT() );
	return ret;
}

template <typename NodeT, typename ParamT> boost::shared_ptr<NodeT> create_node( ParamT par ){
	BOOST_STATIC_ASSERT( ( std::tr1::is_base_of<node, NodeT>::value ) );
	boost::shared_ptr<NodeT> ret( new NodeT(par) );
	return ret;
}

template <typename NodeT>
inline boost::shared_ptr<NodeT> create_node(
	boost::shared_ptr<token_t> const& token_beg,
	boost::shared_ptr<token_t> const& token_end
	)
{
	BOOST_STATIC_ASSERT( ( std::tr1::is_base_of<node, NodeT>::value ) );
	boost::shared_ptr<NodeT> ret( new NodeT(token_beg, token_end) );
	return ret;
}

END_NS_SASL_SYNTAX_TREE();

#endif