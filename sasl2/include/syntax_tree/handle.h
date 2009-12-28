#ifndef SASL_SYNTAX_TREE_HANDLE_H
#define SASL_SYNTAX_TREE_HANDLE_H

#include "../../enums/syntax_node_types.h"
#include <boost/smart_ptr.hpp>

template <typename T>
struct node_tag{
	typedef T type;
};

struct node;
struct token_attr;

template <typename NodeT> struct node_handle_impl;
typedef node_handle_impl<token_attr> token_attr_handle;

struct node_handle{
	boost::shared_ptr<node> body;

	node_handle(){}
	node_handle( const boost::shared_ptr<node> body)
		: body(body){}
	node_handle( const node_handle& rhs)
		:body(rhs.body){}

	template <typename NodeT>
	boost::shared_ptr<NodeT> get_body( bool create_if_need ) const{
		if ( !body && create_if_need ){
			const_cast< node_handle& >(*this).body.reset( new NodeT() );
		}
		return boost::shared_polymorphic_downcast<NodeT>(body);
	}
};

template <typename NodeT>
struct node_handle_impl : public node_handle{

	node_handle_impl<NodeT>& self_(){ return *this; }

	// default constructor
	node_handle_impl(){}
	
	// copy constructors
	node_handle_impl( const node_handle& rhs)
		: node_handle( rhs ){}
	node_handle_impl( const node_handle_impl<NodeT>& rhs )
		: node_handle( rhs ){}

	// normal constructor
	node_handle_impl( NodeT* p ): node_handle( boost::shared_ptr<NodeT>(p) ){}

	// constructor for token_attr
	template< typename IteratorT > node_handle_impl( const IteratorT& first, const IteratorT& last ){}

	// conversation
	template <typename T> explicit node_handle_impl( const T& val ){ NodeT::convert_from( self_(), val ); }
	template <typename T> operator T& () { return NodeT::convert_to( *this, node_tag<T>() ); }
	template <typename T> operator const T& () const{ return NodeT::convert_to( *this, node_tag<T>() );	}

	node_handle_impl<NodeT>& operator = (const node_handle_impl<NodeT>& rhs){
		body = rhs.body;
		return *this;
	}

	NodeT* operator ->() const{
		return get_body<NodeT>( true ).get();
	}
};

#endif // SASL_SYNTAX_TREE_HANDLE_H