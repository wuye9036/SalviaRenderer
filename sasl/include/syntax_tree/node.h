#ifndef SASL_SYNTAX_TREE_NODE_H
#define SASL_SYNTAX_TREE_NODE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/common/token.h>
#include <sasl/enums/node_ids.h>

#include <eflib/include/utility/shared_declaration.h>

#include <any>
#include <vector>
#include <memory>

namespace sasl{
	namespace common{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(token_t);
	}
	namespace semantic{
		EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);
		EFLIB_DECLARE_CLASS_SHARED_PTR(node_semantic);
	}
	namespace codegen{
		EFLIB_DECLARE_CLASS_SHARED_PTR(module_vmcode);
	}
}

#define SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL() void accept( syntax_tree_visitor*, ::std::any* data )
#define SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( node_class_name ) \
	void node_class_name ::accept ( syntax_tree_visitor* v, ::std::any* data ) \
{	\
	v->visit( *this, data );\
}
BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_visitor;

EFLIB_USING_SHARED_PTR(sasl::common, token_t);

EFLIB_DECLARE_STRUCT_SHARED_PTR(node);

struct node: public std::enable_shared_from_this<node>{
	friend class swallow_duplicator;
	friend class deep_duplicator;

	node_ptr as_handle() const;
	template <typename T> std::shared_ptr<T> as_handle() const
	{
		return std::dynamic_pointer_cast<T>( as_handle() );
	}

	token_t_ptr	token_begin() const;
	token_t_ptr	token_end() const;
	void		token_range(token_t_ptr const& tok_beg, token_t_ptr const& tok_end);
	node_ids	node_class() const;

	virtual SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL() = 0;

protected:
	node(node_ids tid, token_t_ptr const& tok_beg, token_t_ptr const& tok_end);
	node& operator = ( const node& ) = delete;
	node( const node& ) = delete;

	node_ids	type_id;
	token_t_ptr	tok_beg, tok_end;

	virtual ~node();
};

//template <typename NodeT> std::shared_ptr<NodeT> create_node();
//template <typename R, typename P0> std::shared_ptr<R> create_node( P0 ); 
//template <typename R, typename P0, typename P1> std::shared_ptr<R> create_node( P0, P1 );

END_NS_SASL_SYNTAX_TREE();

#define SASL_SYNTAX_NODE_CREATORS() \
	template <typename R> friend std::shared_ptr<R> create_node(); \
	template <typename R, typename P0> friend std::shared_ptr<R> create_node( P0&& ); \
	template <typename R> friend std::shared_ptr<R> create_node(std::shared_ptr<token_t> const&, std::shared_ptr<token_t> const&);

	
#endif //SASL_SYNTAX_TREE_NODE_H
