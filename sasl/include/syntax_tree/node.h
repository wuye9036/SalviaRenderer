#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/common/token.h>
#include <sasl/enums/node_ids.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/pointee.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace common{
		struct token_t;
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

using ::sasl::common::token_t;

struct node{
	friend class swallow_duplicator;
	friend class deep_duplicator;

	boost::shared_ptr<node> as_handle() const;
	template <typename T> boost::shared_ptr<T> as_handle() const{
		return boost::shared_polymorphic_cast<T>( as_handle() );
	}

	boost::shared_ptr<class ::sasl::semantic::symbol> symbol() const;
	void symbol( boost::shared_ptr<class ::sasl::semantic::symbol> sym );

	boost::shared_ptr<class ::sasl::semantic::semantic_info> semantic_info() const;
	void semantic_info( boost::shared_ptr<class ::sasl::semantic::semantic_info> ) const;

	template <typename T> T* si_ptr() const{
#ifdef EFLIB_DEBUG
		return dyn_siptr<T>();
#else
		return static_cast<T*>( semantic_info().get() );
#endif
	}
	
	template <typename T> T* dyn_siptr() const{
		if( seminfo ){ 
			T* ptr = dynamic_cast<T*>( semantic_info().get() );
			assert( ptr );
			return ptr;
		} else {
			return NULL;
		}
	}

	boost::shared_ptr<token_t>	token_begin() const;
	boost::shared_ptr<token_t>	token_end() const;
	void						token_range( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	node_ids					node_class() const;

	virtual SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL() = 0;

protected:
	node(node_ids tid, boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end);
	node& operator = ( const node& );
	node( const node& );

	node_ids										type_id;
	boost::shared_ptr<token_t>						tok_beg, tok_end;
	boost::weak_ptr<class sasl::semantic::symbol>	sym;

	boost::shared_ptr<class sasl::semantic::semantic_info>			seminfo;
	boost::shared_ptr<class sasl::code_generator::codegen_context>	cgctxt;

	boost::weak_ptr<node> selfptr;

	virtual ~node();
};

//template <typename NodeT> boost::shared_ptr<NodeT> create_node();
//template <typename R, typename P0> boost::shared_ptr<R> create_node( P0 ); 
//template <typename R, typename P0, typename P1> boost::shared_ptr<R> create_node( P0, P1 );

END_NS_SASL_SYNTAX_TREE();

#define SASL_SYNTAX_NODE_CREATORS() \
	template <typename R> friend boost::shared_ptr<R> create_node(); \
	template <typename R, typename P0> friend boost::shared_ptr<R> create_node( P0 ); \
	template <typename R, typename P0, typename P1> friend boost::shared_ptr<R> create_node( P0, P1 );

	
#endif //SASL_SYNTAX_TREE_NODE_H
