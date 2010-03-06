DEFINE_TREE_NODE_PROCESSOR( r_scalar_type )
{
	const typename tree_node_t::parse_node_t& value = it->value;
	const typename tree_node_t::children_t& children = it->children;

	h_ast_node_scalar_type ret_node = ast_node_scalar_type::create( string(value.begin(), value.end()) );
	return_node( ret_node );
}

DEFINE_TREE_NODE_PROCESSOR( r_vector ){
	typename const tree_node_t::parse_node_t& value = it->value;
	typename const tree_node_t::children_t& children = it->children;

	h_ast_node_scalar_type scalar_node;
	get_node( scalar_node, process_rule_node( children.begin() ) );

	return_node( ast_node_vector_type::create( 
		scalar_node, 
		ast_node_literal_expression::create( get_child_string(it, 1), "", literal_types::integer ) 
		) );
}

DEFINE_TREE_NODE_PROCESSOR( r_matrix ){
	typename const tree_node_t::children_t& children = it->children;

	h_ast_node_scalar_type scalar_node;
	get_node( scalar_node, process_rule_node( iter_at( children, 0 ) ) );

	return_node( ast_node_matrix_type::create(
		scalar_node,
		ast_node_literal_expression::create( get_child_string(it, 1), "", literal_types::integer ),
		ast_node_literal_expression::create( get_child_string(it, 2), "", literal_types::integer )
		) );
}

DEFINE_TREE_NODE_PROCESSOR( r_buildin_type ){
	return_node( process_rule_node( it ) );
}