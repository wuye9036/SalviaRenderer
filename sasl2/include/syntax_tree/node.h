#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include "handle.h"

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

template <typename DerivedT>
struct node_impl: public node{
	typedef DerivedT this_type;
	typedef node_impl<DerivedT> base_type;
	typedef node_handle_impl<DerivedT> handle_t;

	node_impl(syntax_node_types type, const token_attr_handle& tok)
		: node(type, tok){}
};

template <typename DerivedT>
struct non_terminal: public node_impl<DerivedT>{
	non_terminal( syntax_node_types type, const token_attr_handle& tok )
		:base_type( type, tok ){}
};

template <typename DerivedT>
struct terminal: public node_impl<DerivedT>{
	terminal( syntax_node_types type, const token_attr_handle& tok )
		:base_type( type, tok ){}

	static void convert_from( handle_t& hdl, const token_attr_handle& v ){
		if (v.body){
			hdl.get_body<this_type>( true )->tok = v;
		}
	}

	static token_attr_handle& convert_to( const handle_t& hdl, node_tag<token_attr_handle>& ){
		return hdl->tok;
	}
};

#endif //SASL_SYNTAX_TREE_NODE_H