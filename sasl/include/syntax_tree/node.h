#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/common/token.h>
#include <sasl/enums/node_ids.h>

#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/pointee.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace common{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(token_t);
	}
	namespace semantic{
		EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);
		EFLIB_DECLARE_CLASS_SHARED_PTR(semantic_info);
	}
	namespace code_generator{
		EFLIB_DECLARE_CLASS_SHARED_PTR(codegen_context);
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

EFLIB_USING_SHARED_PTR(sasl::common, token_t);

EFLIB_DECLARE_STRUCT_SHARED_PTR(node);

struct node: public boost::enable_shared_from_this<node>{
	friend class swallow_duplicator;
	friend class deep_duplicator;

	node_ptr as_handle() const;
	template <typename T> boost::shared_ptr<T> as_handle() const
	{
		return boost::shared_dynamic_cast<T>( as_handle() );
	}

	sasl::semantic::symbol_ptr symbol() const;
	void symbol(sasl::semantic::symbol_ptr);

	sasl::semantic::semantic_info_ptr semantic_info() const;
	void semantic_info(sasl::semantic::semantic_info_ptr) const;

	template <typename T> T* si_ptr() const{
#ifdef EFLIB_DEBUG
		T* ret = dyn_siptr<T>();
		if ( !ret && semantic_info() ){
			assert(false);
		}
		return ret;
#else
		return static_cast<T*>( semantic_info().get() );
#endif
	}
	
	template <typename T> T* dyn_siptr() const{
		T* ptr = dynamic_cast<T*>( semantic_info().get() );
		return ptr;
	}

	token_t_ptr	token_begin() const;
	token_t_ptr	token_end() const;
	void		token_range(token_t_ptr const& tok_beg, token_t_ptr const& tok_end);
	node_ids	node_class() const;

	virtual SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL() = 0;

protected:
	node(node_ids tid, token_t_ptr const& tok_beg, token_t_ptr const& tok_end);
	node& operator = ( const node& );
	node( const node& );

	node_ids	type_id;
	token_t_ptr	tok_beg, tok_end;
	boost::weak_ptr<class sasl::semantic::symbol>	sym;

	sasl::semantic::semantic_info_ptr			seminfo;
	sasl::code_generator::codegen_context_ptr	cgctxt;

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
