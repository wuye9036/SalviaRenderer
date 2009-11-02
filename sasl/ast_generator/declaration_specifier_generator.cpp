template< typename TreeIteratorT >
h_ast_node_type ast_tree_generator< TreeIteratorT >::qualify_type(TreeIteratorT& type_expr_it, TreeIteratorT& quals_begin, TreeIteratorT& quals_end ){
	h_ast_node_type current_type;
	get_node( current_type, process_rule_node(type_expr_it) );
	
	for(TreeIteratorT it_qual = quals_begin; it_qual != quals_end; ++it_qual ){
		size_t qual_rule_id = it_qual->value.id().to_long();

		switch( qual_rule_id ){
			case parser_ids::r_function_qualifier:{
					// function parameter type is sub nodes of function qualifier.

					// get function parameter type lists.
					vector<h_ast_node_variable_declaration> decls;
					for( size_t i_decl_spec = 0; i_decl_spec < it_qual->children.size(); ++i_decl_spec ){
						// get decl spec
						h_ast_node_type decl_spec_node;
						get_node( decl_spec_node, get_child_ast_node(it_qual, i_decl_spec) );

						//convert decl spec list to unnamed declaration
						h_ast_node_variable_declaration decl_node = ast_node_variable_declaration::create( decl_spec_node, h_ast_node_identifier(), h_ast_node_expression() );

						//add to decl list
						decls.push_back( decl_node );
					}

					get_node( current_type, ast_node_function::create(current_type, decls, h_ast_node_statement()) ) ;
				}
				break;

			case parser_ids::r_array_qualifier:	{
					h_ast_node_expression size_expr;
					get_node( size_expr, process_rule_node( it_qual ) );

					if( current_type->get_node_type() == ast_node_types::array_type ){
						h_ast_node_array current_array_type;
						get_node( current_array_type, current_type );
						current_array_type->get_size_expressions().push_back( size_expr ); 
					} else {
						vector<h_ast_node_expression> exprs;
						exprs.push_back( size_expr );
						get_node( current_type, ast_node_array::create(current_type, exprs) );
					}
				}
				break;

			case parser_ids::r_keyword_qualifier: {
					h_ast_node_type_qualifier type_qual;
					get_node( type_qual, process_rule_node( it_qual ) );
					current_type->get_type_qualifiers().push_back( type_qual->get_qualifier() );
				}
				break;
		}
	}

	return current_type;
}

DEFINE_TREE_NODE_PROCESSOR( r_decl_spec ){
	return_node( get_child_ast_node( it, 0 ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_postfix_qualified_type ){
	const children_t& children = it->children;
	return_node( 
		qualify_type( children.begin(), children.begin() + 1, children.end() ) 
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_prefix_qualified_type ){
	const children_t& children = it->children;
	children_t reverse_children( it->children.rbegin(), it->children.rend() );
	return_node(
		qualify_type( reverse_children.begin(), reverse_children.begin() + 1, reverse_children.end() )
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_prefix_type_qualifier ){
	return_node( get_child_ast_node(it, 0) );
}
DEFINE_TREE_NODE_PROCESSOR( r_postfix_type_qualifier ){
	return_node( get_child_ast_node(it, 0) );
}
DEFINE_TREE_NODE_PROCESSOR( r_keyword_qualifier ){
	return_node(
		ast_node_identifier::create( string( it->value.begin(), it->value.end() ) )
	);
}
DEFINE_TREE_NODE_PROCESSOR( r_function_qualifier ){
	//NOTE: THIS FUNCTION IS NOT EXECUTED.
	return_node( h_ast_node() );
}
DEFINE_TREE_NODE_PROCESSOR( r_array_qualifier ){
	//NOTE: THIS FUNCTION IS NOT EXECUTED.
	return_node( h_ast_node() );
	//if( it->children.empty() ){
	//	return_node( h_ast_node_expression() );
	//} else {
	//	return_node( get_child_ast_node( it, 0 ) );
	//}
}
