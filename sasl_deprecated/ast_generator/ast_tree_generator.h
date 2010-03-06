#ifndef SASL_AST_TREE_GENERATOR_H
#define SASL_AST_TREE_GENERATOR_H

#include "utilities.h"
#include "../parsers/parser_ids.h"
#include "../utility/declare_handle.h"

#include <vector>

DECL_HANDLE( ast_node );
DECL_HANDLE( ast_node_type );

template<typename TreeIteratorT>
class ast_tree_generator{
public:
	ast_tree_generator();
	void generate(TreeIteratorT& it);
	h_ast_node root();

private:
	typedef typename std::iterator_traits<TreeIteratorT>::value_type tree_node_t;
	typedef typename tree_node_t::children_t children_t;
	typedef TreeIteratorT tree_iterator_t;
	typedef typename tree_node_t::parse_node_t value_t;

	DECL_TREE_NODE_PROCESSORS( PARSER_SEQ (r_lcombine_binary_expr) );//所有左结合运算符表达式树的生成方法。
	h_ast_node process_rule_node( TreeIteratorT& it );
	h_ast_node get_child_ast_node( TreeIteratorT& it_parent, size_t i_child );
	template<typename HOutNodeT>
	void get_child_ast_node(HOutNodeT& hout, TreeIteratorT& it_parent, size_t i_child);

	//将一组子节点转化为ast_node的Concret Class的句柄并追加到容器中。
	template< typename PushBackableContainerT >
		void get_child_ast_node_list( PushBackableContainerT& cont, TreeIteratorT& it_parent, size_t i_child_begin, size_t i_child_end );
	h_ast_node_type qualify_type(TreeIteratorT& type_expr_it, TreeIteratorT& quals_begin, TreeIteratorT& quals_end );

	typedef h_ast_node (ast_tree_generator<TreeIteratorT>::*tree_node_process_functor) (TreeIteratorT& tree_node);
	std::vector<tree_node_process_functor> processors_;
	h_ast_node root_;
};

#endif