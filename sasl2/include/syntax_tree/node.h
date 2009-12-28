#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include "../../enums/syntax_node_types.h"
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>
#include <vector>
#include <boost/mpl/vector.hpp>

struct node;
struct node_handle{
	boost::shared_ptr<node> body;

	node_handle(){}
	node_handle( const boost::shared_ptr<node> body)
		: body(body){}
	node_handle( const node_handle& rhs)
		:body(rhs.body){}

	template <typename NodeT>
	boost::shared_ptr<NodeT> get_body() const{
		if ( !body ){
			const_cast< node_handle& >(*this).body.reset( new NodeT() );
		}
		return boost::shared_polymorphic_downcast<NodeT>(body);
	}
};

struct token_attr;
template <typename NodeT> struct terminal_handle;
typedef terminal_handle<token_attr> token_attr_handle;

template <typename NodeT>
struct node_handle_impl : public node_handle{
	NodeT* operator ->() const{
		return get_body<NodeT>().get();
	}

	template< typename IteratorT >
	node_handle_impl( const IteratorT& first, const IteratorT& last ){
	}

	node_handle_impl(){}
	node_handle_impl( NodeT* p )
		: node_handle( boost::shared_ptr<NodeT>(p) ){}
	node_handle_impl( const node_handle& rhs)
		: node_handle( rhs ){}
	node_handle_impl( const node_handle_impl<NodeT>& rhs )
		: node_handle( rhs ){}
};

template <typename NodeT>
struct terminal_handle : public node_handle_impl<NodeT>{
	terminal_handle()
		: node_handle_impl<NodeT>(){}
	terminal_handle( NodeT* p )
		: node_handle_impl<NodeT>(p){}
	terminal_handle( const token_attr_handle& tok ){
		if (tok.body){
			get_body<NodeT>()->tok = tok;
		}
	}

	operator token_attr_handle& () {
		return get_body<NodeT>()->tok;
	}

	terminal_handle<NodeT>& operator = (const terminal_handle<NodeT>& rhs){
		body = rhs.body;
		return *this;
	}
};

template <>
struct terminal_handle<token_attr>: public node_handle_impl<token_attr>{
	terminal_handle()
		: node_handle_impl<token_attr>(){}
	terminal_handle( token_attr* p )
		: node_handle_impl<token_attr>(p){}
	template< typename IteratorT >
	terminal_handle( const IteratorT& first, const IteratorT& last ){ }
	terminal_handle( const token_attr_handle& tok )
		: node_handle_impl<token_attr>( tok ) {}

	operator token_attr_handle& () {
		return *this;
	}

	terminal_handle<token_attr>& operator = (const terminal_handle<token_attr>& rhs){
		body = rhs.body;
		return *this;
	}
};

struct node{

	syntax_node_types type;
	token_attr_handle tok;
	
	virtual void update() = 0;

protected:
	node(syntax_node_types type, const token_attr_handle& tok)
		: type(type), tok(tok){
	}
	virtual ~node(){}
};

template <typename DerivedT> struct terminal;

template <typename DerivedT>
struct node_impl: public node{
	typedef node_impl<DerivedT> this_type;
	typedef node_impl<DerivedT> base_type;

	node_impl(syntax_node_types type, const token_attr_handle& tok)
		: node(type, tok){}
};

template <typename DerivedT>
struct non_terminal: public node_impl<DerivedT>{
	typedef node_handle_impl<DerivedT> handle_t;
	non_terminal( syntax_node_types type, const token_attr_handle& tok )
		:base_type( type, tok ){}
};

template <typename DerivedT>
struct terminal: public node_impl<DerivedT>{
	typedef terminal_handle<DerivedT> handle_t;
	terminal( syntax_node_types type, const token_attr_handle& tok )
		:base_type( type, tok ){}
};

#endif //SASL_SYNTAX_TREE_NODE_H