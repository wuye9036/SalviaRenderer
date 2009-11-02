#ifndef SASL_AST_GENERATOR_UTILITY_MACROS_H
#define SASL_AST_GENERATOR_UTILITY_MACROS_H

///////////////////////////////////////////
//  get processor name from a rule name  //
///////////////////////////////////////////
#define PROCESSOR_OF( rule_name ) BOOST_PP_SEQ_CAT( (process_) (rule_name) (_node) )

////////////////////////////////////
//  declare tree node processors  //
////////////////////////////////////
#define DECL_TREE_NODE_PROCESSOR_ITEM( r, data, rule_name ) h_ast_node PROCESSOR_OF(rule_name) ( TreeIteratorT& it);
#define DECL_TREE_NODE_PROCESSORS( rule_seq ) BOOST_PP_SEQ_FOR_EACH( DECL_TREE_NODE_PROCESSOR_ITEM, 0, rule_seq );

////////////////////////////////////
//  define a tree node processor  //
////////////////////////////////////
#define DEFINE_TREE_NODE_PROCESSOR( rule_name )\
	template<typename TreeIteratorT> \
	h_ast_node BOOST_PP_CAT( ast_tree_generator<TreeIteratorT>::, PROCESSOR_OF( rule_name ) ) (TreeIteratorT& it)

//////////////////////////////////////////////////////////////////
// give a default definition of rule processor.	                //
// the default implementation return a null handle of ast_node. //
//////////////////////////////////////////////////////////////////
#define DEFINE_DEFAULT_TREE_NODE_PROCESSOR( rule_name )\
	DEFINE_TREE_NODE_PROCESSOR(rule_name){ return h_ast_node(); }
#define DEFINE_DEFAULT_TREE_NODE_PROCESSOR_ITEM( r, data, item) DEFINE_DEFAULT_TREE_NODE_PROCESSOR( item )
#define DEFINE_DEFAULT_TREE_NODE_PROCESSORS( rule_seq ) BOOST_PP_SEQ_FOR_EACH( DEFINE_DEFAULT_TREE_NODE_PROCESSOR_ITEM, 0, rule_seq )

///////////////////////////////////////////////////////////
//  support a left-combined binary expression processor  //
///////////////////////////////////////////////////////////
#define DEFINE_LCOMB_BINARY_EXPRESSION_PROCESSOR_ITEM( r, data, item ) DEFINE_TREE_NODE_PROCESSOR(item){ return_node( CALL_PROCESSOR( r_lcombine_binary_expr, it ) ); }
#define DEFINE_LCOMB_BINARY_EXPRESSION_PROCESSORS( rule_seq ) BOOST_PP_SEQ_FOR_EACH( DEFINE_LCOMB_BINARY_EXPRESSION_PROCESSOR_ITEM, 0, rule_seq )

/////////////////////////////////////
//  processors table initializers  //
/////////////////////////////////////
#define INITIALIZE_TREE_NODE_PROCESSOR( rule_name )\
	processors_.push_back( & BOOST_PP_CAT( ast_tree_generator<TreeIteratorT>::, PROCESSOR_OF( rule_name ) ) );
#define INITIALIZE_TREE_NODE_PROCESSOR_TABLE_ITEM( r, data, item ) INITIALIZE_TREE_NODE_PROCESSOR( item );
#define INITIALIZE_TREE_NODE_PROCESSOR_TABLE( rule_seq ) BOOST_PP_SEQ_FOR_EACH( INITIALIZE_TREE_NODE_PROCESSOR_TABLE_ITEM, 0, rule_seq );

////////////////////////
//  call a processor  //
////////////////////////
#define CALL_PROCESSOR( rule_name, it ) PROCESSOR_OF( rule_name ) (it)

//////////////////////////////////////////////////////
//  handle of ast node conversation and assignment  //
//////////////////////////////////////////////////////
#define return_node( node )\
	return static_pointer_cast<ast_node>( (node) );

///////////////////////////
//  some help functions  //
///////////////////////////
#include <boost/pointer_cast.hpp>

#include <string>

template< typename TreeIteratorT >
std::string get_child_string( TreeIteratorT& it_parent, size_t i_child ){
	return std::string( 
		value_at( it_parent->children, i_child ).begin(),
		value_at( it_parent->children, i_child ).end()
		);
}

template<typename HOutNodeT, typename HInNodeT>
HOutNodeT& get_node( HOutNodeT& dest_node, HInNodeT src_node){
	return dest_node = boost::static_pointer_cast< typename HOutNodeT::element_type >( src_node );
}

template<typename ContainerT>
typename ContainerT::const_iterator iter_at( const ContainerT& cont, size_t i){
	return cont.begin() + i;
}

template<typename ContainerT>
typename const ContainerT::value_type::parse_node_t& value_at( const ContainerT& cont, size_t i){
	return iter_at(cont, i)->value;
}

template<typename ContainerT>
void reverse_container(ContainerT& out_cont, const ContainerT& in_cont){
	out_cont.clear();
	for( typename ContainerT::const_reverse_iterator r_it = in_cont.rbegin(); r_it != in_cont.rend(); ++r_it ){
		out_cont.push_back( *r_it );
	}
}

#endif