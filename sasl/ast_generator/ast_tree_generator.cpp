#include "ast_tree_generator.h"

#include "operation_symbols.h"

#include "../ast_nodes/ast_node.h"
#include "../ast_nodes/ast_node_buildin_type.h"
#include "../ast_nodes/ast_node_expression.h"
#include "../ast_nodes/ast_node_type_qualifier.h"
#include "../ast_nodes/ast_node_literal_expression.h"

#include <boost/spirit/include/classic_ast.hpp>
#include <boost/pointer_cast.hpp>

using namespace boost;
using namespace boost::spirit::classic;

using namespace std;

//将当前节点的第n个子节点解析为AST Node并返回
template< typename TreeIteratorT >
ast_tree_generator<TreeIteratorT>::ast_tree_generator(){
	INITIALIZE_TREE_NODE_PROCESSOR_TABLE( PARSER_SEQ (r_lcombine_binary_expr) );
}

///////////////////////
//  other functions  //
///////////////////////
template<typename TreeIteratorT>
void ast_tree_generator<TreeIteratorT>::generate(TreeIteratorT &it){
	root_ = process_rule_node( it );
}

template<typename TreeIteratorT>
h_ast_node ast_tree_generator<TreeIteratorT>::root(){
	return root_;
}

template<typename TreeIteratorT>
h_ast_node ast_tree_generator<TreeIteratorT>::get_child_ast_node( TreeIteratorT& it_parent, size_t i_child ){
	return_node( process_rule_node( iter_at(it_parent->children, i_child) ) );
}
template<typename TreeIteratorT>
template<typename HOutNodeT>
void ast_tree_generator<TreeIteratorT>::get_child_ast_node(HOutNodeT& hout, TreeIteratorT& it_parent, size_t i_child){
	get_node( hout, get_child_ast_node(it_parent, 0) );
}

template< typename TreeIteratorT>
template< typename PushBackableContainerT >
void ast_tree_generator<TreeIteratorT>::get_child_ast_node_list(
	PushBackableContainerT& cont,
	TreeIteratorT& it_parent,
	size_t i_child_begin, size_t i_child_end 
	)
{
	typedef typename PushBackableContainerT::value_type handle_type;
	for( size_t i_child = i_child_begin; i_child < i_child_end; ++i_child ){
		handle_type current_node;
		get_node( current_node, get_child_ast_node( it_parent, i_child ) );
		cont.push_back( current_node );
	}
}
///////////////////////
//  rule processors  //
///////////////////////

DEFINE_TREE_NODE_PROCESSOR( rule ){
	size_t parser_id = it->value.id().to_long();
	return ( this->*(processors_[parser_id]) ) (it);
}

//////////////////////////////////////////////
//  some common rule processor definitions  //
//////////////////////////////////////////////
DEFINE_TREE_NODE_PROCESSOR( r_identifier ){
	typename const tree_node_t::parse_node_t& value = it->value;
	return_node( ast_node_identifier::create( string( value.begin(), value.end() ) ) );
}

DEFINE_DEFAULT_TREE_NODE_PROCESSORS(
	(r_program)
	(r_whitespace)
	(r_newline_space)
 	(r_type_suffix)
);

////////////////////
//  SANDBOX ZONE  //
////////////////////
#include "statement_generator.cpp"
#include "expression_generator.cpp"
#include "buildin_type_generator.cpp"
#include "declaration_specifier_generator.cpp"
#include "declaration_generator.cpp"

/////////////////////
//  Instantiation  //
/////////////////////
h_ast_node instantiate_tree_generator(){
	ast_tree_generator< tree_match<char*>::node_t::const_tree_iterator > gen;
	gen.generate( tree_match<char*>().trees.begin() ) ;
	h_ast_node ret_node = gen.root();
	return ret_node;
}