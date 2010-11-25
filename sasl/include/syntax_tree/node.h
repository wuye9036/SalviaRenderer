#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/enums/syntax_node_types.h>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

namespace sasl{ 
	namespace common{ 
		struct token_attr; 
	}

	namespace semantic{
		class symbol;
		class semantic_info;
	}

	namespace code_generator{
		class codegen_context;
	}
}
#define SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL() void accept( syntax_tree_visitor*, ::boost::any* data )
#define SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( node_class_name ) \
	void node_class_name ::accept ( syntax_tree_visitor* v, ::boost::any* data ) \
{	\
	v->visit( *this, data );\
}
BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_visitor;

using ::sasl::common::token_attr;

struct node{
	friend class swallow_duplicator;
	friend class deep_duplicator;

	boost::shared_ptr<node> handle() const;
	template <typename T> boost::shared_ptr<T> typed_handle(  ) const{
		return boost::shared_polymorphic_cast<T>( handle() );
	}

	boost::shared_ptr<class ::sasl::semantic::symbol> symbol() const;
	void symbol( boost::shared_ptr<class ::sasl::semantic::symbol> sym );

	boost::shared_ptr<class ::sasl::semantic::semantic_info> semantic_info() const;
	void semantic_info( boost::shared_ptr<class ::sasl::semantic::semantic_info> ) const;

	boost::shared_ptr<class ::sasl::code_generator::codegen_context> codegen_ctxt() const;
	void codegen_ctxt( boost::shared_ptr<class ::sasl::code_generator::codegen_context> ) const;

	boost::shared_ptr<token_attr> token() const;
	syntax_node_types node_class() const;

	virtual SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL() = 0;

	const ::std::vector< ::boost::shared_ptr< node > >& additionals() const;
	::std::vector< ::boost::shared_ptr< node > >& additionals();

protected:
	node(syntax_node_types tid, boost::shared_ptr<token_attr> tok);
	node& operator = ( const node& );
	node( const node& );

	syntax_node_types				type_id;
	boost::shared_ptr<token_attr>	tok;
	boost::shared_ptr<class ::sasl::semantic::symbol>	sym;
	boost::shared_ptr<class ::sasl::semantic::semantic_info> seminfo;
	boost::shared_ptr<class ::sasl::code_generator::codegen_context> cgctxt;
	boost::weak_ptr<node> selfptr;

	//	additional syntax nodes.
	//		It makes no sense in original syntax tree,
	//		but could use when evaluate syntax tree processing.
	//		It only hold life time of syntax node utility, and
	//		without any processing during the syntax tree is visited.
	::std::vector< ::boost::shared_ptr< node > > adds;

	virtual ~node();
};

template <typename NodeT> boost::shared_ptr<NodeT> create_node();
template <typename NodeT, typename ParamT> boost::shared_ptr<NodeT> create_node(ParamT);

END_NS_SASL_SYNTAX_TREE();

#define SASL_SYNTAX_NODE_CREATORS() \
	template <typename R> friend boost::shared_ptr<R> create_node(); \
	template <typename R, typename P0> friend boost::shared_ptr<R> create_node( P0 );

#endif //SASL_SYNTAX_TREE_NODE_H