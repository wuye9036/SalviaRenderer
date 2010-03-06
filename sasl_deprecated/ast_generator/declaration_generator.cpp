#include "../ast_nodes/ast_node_declaration.h"
#include "../ast_nodes/ast_node_statement.h"
#include <boost/foreach.hpp>

typedef std::vector<h_ast_node_initialized_declarator> init_decl_list_t;
typedef handle_type_of< ast_node_value_expression<init_decl_list_t> >::result init_decl_list_ast_node_t;

DEFINE_TREE_NODE_PROCESSOR( r_declaration ){
	return_node( get_child_ast_node(it, 0) );
}

DEFINE_TREE_NODE_PROCESSOR( r_block_declaration ){
	h_ast_node_type decl_spec_node;
	init_decl_list_ast_node_t init_decl_list_node;

	//get declaration specifier
	get_node( decl_spec_node, get_child_ast_node(it, 0) );

	//get variable name and its initializer.
	get_node( init_decl_list_node, get_child_ast_node( it, 1) );

	//make declaration list by assign declaration specifier to each declarator
	init_decl_list_t init_decl_list = init_decl_list_node->get_value();
	vector< h_ast_node_variable_declaration > decl_list;
	BOOST_FOREACH( h_ast_node_initialized_declarator init_decl_node, init_decl_list ){
		h_ast_node_variable_declaration decl_node = ast_node_variable_declaration::create(
			decl_spec_node, 
			init_decl_node->get_identifier(),
			init_decl_node->get_initializer() 
			);
		decl_list.push_back( decl_node );
	}

	// composite declaration list to block declaration
	return_node(
		ast_node_block_declaration::create( decl_list )
		);
}

DEFINE_TREE_NODE_PROCESSOR( r_function_definition ){
	h_ast_node_type decl_spec_node;
	h_ast_node_identifier ident_node;
	h_ast_node_block_declaration params_block_node;
	h_ast_node_statement body_node;
	vector<h_ast_node_variable_declaration> params_node;

	get_node( decl_spec_node, get_child_ast_node( it, 0 ) );
	get_node( ident_node, get_child_ast_node(it, 1) );
	get_node( params_block_node, get_child_ast_node(it, 2) );

	//if has function body, there are 4 children of function definition
	if( it->children.size() == 4 ){
		get_node( body_node, get_child_ast_node(it, 3) );
	}

	//composite function type
	h_ast_node_type func_type;
	get_node( 
		func_type,
		ast_node_function::create( decl_spec_node, params_block_node->get_declarations(), body_node )
		);

	//composite function type and identifier
	return_node(
		ast_node_variable_declaration::create( func_type, ident_node, h_ast_node_expression() )
		);
}

DEFINE_TREE_NODE_PROCESSOR( r_parameter_item ){
	h_ast_node_type param_type_node;
	h_ast_node_identifier ident_node;
	h_ast_node_expression default_expr_node;

	get_node( param_type_node, get_child_ast_node(it, 0) );

	if( it->children.size() > 1 ){
		//param has a name
		get_node( ident_node, get_child_ast_node(it, 1) );
	}

	if( it->children.size() > 2 ){
		//param has a defalut value
		get_node( default_expr_node, get_child_ast_node(it, 2) );
	}

	return_node(
		ast_node_variable_declaration::create( param_type_node, ident_node, default_expr_node )
		);
}

DEFINE_TREE_NODE_PROCESSOR( r_parameters ){
	vector< h_ast_node_variable_declaration > param_decls;
	for(size_t i_child = 0; i_child < it->children.size(); ++i_child ){
		h_ast_node_variable_declaration param_decl_node;
		get_node( param_decl_node, get_child_ast_node(it, i_child) );
		param_decls.push_back( param_decl_node );
	}

	return_node(
		ast_node_block_declaration::create( param_decls )
		);
}

DEFINE_TREE_NODE_PROCESSOR( r_struct_definition ){
	//definition includes only a struct type.
	return_node(
		get_child_ast_node( it, 0 )
		);
}

DEFINE_TREE_NODE_PROCESSOR( r_unqualified_type ){
	return_node( get_child_ast_node( it, 0 ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_identifier_type ){
	h_ast_node_identifier ident;
	get_node( ident, get_child_ast_node(it, 0) );

	return_node( ast_node_identifier_type::create( ident ) ); 
}

DEFINE_TREE_NODE_PROCESSOR( r_initializer ){
	return_node( get_child_ast_node( it, 0 ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_initialize_expr ){
	vector< h_ast_node_expression > exprs;
	
	for( size_t i_child = 0; i_child < it->children.size(); ++i_child ){
		h_ast_node_expression child_expr;
		get_node( child_expr, get_child_ast_node( it, i_child ) );
		exprs.push_back( child_expr );
	}


	// NOTE: 为什么是
	//	return_node( ast_node_expression_list::create( exprs, true ) );
	// 这样的，当时是怎么考虑这个问题的。。。
	return_node( ast_node_expression_list::create(exprs) );
}

DEFINE_TREE_NODE_PROCESSOR( r_initialized_declarator ){
	//has a initializer
	h_ast_node_expression initializer;
	h_ast_node_identifier ident;

	get_node( ident, get_child_ast_node( it, 0 ) );
	if( it->children.size() > 1 ){
		get_node( initializer, get_child_ast_node( it, 1 ) );
	}

	return_node( ast_node_initialized_declarator::create( ident, initializer ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_initialized_declarator_list ){
	vector< h_ast_node_initialized_declarator > init_decl_lst;

	for( size_t i_child = 0; i_child < it->children.size(); ++i_child ){
		h_ast_node_initialized_declarator current_init_decl;
		get_node( current_init_decl, get_child_ast_node( it, i_child ) );
		init_decl_lst.push_back( current_init_decl );
	}

	return_node( ast_node_value_expression< vector<h_ast_node_initialized_declarator> >::create( init_decl_lst ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_declarator ){
	// extract innermost identifier of "( declarator )", so it is sure that return a identifier.
	return_node( get_child_ast_node( it, 0 ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_structure_type ){
	vector< h_ast_node_declaration > decls;
	h_ast_node_identifier ident;

	get_node( ident, get_child_ast_node( it, 0 ) );
	for( size_t i_child = 1; i_child < it->children.size(); ++i_child ){
		h_ast_node_declaration current_decl;
		get_node( current_decl, get_child_ast_node(it, i_child) );
		decls.push_back( current_decl );
	}

	return_node( ast_node_struct::create ( ident, decls ) );
}