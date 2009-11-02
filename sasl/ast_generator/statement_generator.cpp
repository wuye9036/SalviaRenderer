DEFINE_TREE_NODE_PROCESSOR( r_return_statement ){
	h_ast_node_expression ret_expr;
	if( ! it->children.empty() ){
		get_node( ret_expr, get_child_ast_node( it, 0 ) );
	}
	return_node( ast_node_control_statement::create( control_statements::_return, ret_expr ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_break_statement ){
	return_node( ast_node_control_statement::create( control_statements::_break ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_continue_statement ){
	return_node( ast_node_control_statement::create( control_statements::_continue) );
}

DEFINE_TREE_NODE_PROCESSOR( r_compound_statement ){
	vector< h_ast_node_statement > stmts;
	get_child_ast_node_list( stmts, it, 0, it->children.size() );
	return_node( ast_node_compound_statement::create( stmts ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_while_statement ){
	h_ast_node_expression cond_expr;
	h_ast_node_statement while_body;

	get_node( cond_expr, get_child_ast_node( it, 0 ) );
	get_node( while_body, get_child_ast_node( it, 1 ) );

	return_node( ast_node_while_statement::create( cond_expr, while_body ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_do_while_statement ){
	h_ast_node_statement do_body;
	h_ast_node_expression cond_expr;

	get_node( do_body, get_child_ast_node( it, 0 ) );
	get_node( cond_expr, get_child_ast_node( it, 1 ) );
	
	return_node( ast_node_while_statement::create( cond_expr, do_body ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_for_init_statement ){
	return_node( get_child_ast_node( it, 0 ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_for_statement ){
	h_ast_node_statement init_stmt;
	h_ast_node_expression cond_expr;
	h_ast_node_expression_statement cond_expr_stmt;
	h_ast_node_expression loop_expr;
	h_ast_node_statement for_body;

	get_node( init_stmt, get_child_ast_node( it, 0 ) );
	get_node( cond_expr_stmt, get_child_ast_node( it, 1 ) );
	get_node( cond_expr, cond_expr_stmt->get_expression() );
	//it is means that this statement has a loop expr while there are 4 children node.
	if( it->children.size() == 4 ){
		get_node( loop_expr, get_child_ast_node( it, 2) );
	}
	get_node( for_body, get_child_ast_node( it, it->children.size() - 1 ) );

	return_node( ast_node_for_statement::create( init_stmt, cond_expr, loop_expr, for_body ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_null_statement ){
	return_node( ast_node_expression_statement::create( h_ast_node_expression() ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_if_statement ){
	h_ast_node_expression cond_expr;
	h_ast_node_statement if_stmt;
	h_ast_node_statement else_stmt;

	get_node( cond_expr, get_child_ast_node( it, 0 ) );
	get_node( if_stmt, get_child_ast_node( it, 1 ) );
	if( it->children.size() == 3 ){
		get_node( else_stmt, get_child_ast_node( it, 2) );
	}

	return_node( ast_node_if_statement::create( cond_expr, if_stmt, else_stmt ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_switch_statement ){
	h_ast_node_expression cond_expr;
	vector< h_ast_node_statement > stmts;

	get_node( cond_expr, get_child_ast_node( it, 0 ) );
	get_child_ast_node_list( stmts, it, 1, it->children.size() );

	return_node( ast_node_switch_statement::create( cond_expr, stmts ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_labeled_statement ){
	h_ast_node_label lbl;
	string label_str = get_child_string( it, 0 );
	if( label_str == "case" ){
		h_ast_node_expression expr;
		get_child_ast_node( it, 1 );
		lbl = ast_node_label::create( labels::_case, expr);
	} else if ( label_str == "default" ){
		lbl = ast_node_label::create( labels::_default );
	} else {
		lbl = ast_node_label::create( labels::_identifier, ast_node_identifier::create( label_str ) );
	}

	//NOTE:	this statement maybe a labeled statement.
	//		so label of statement will be found recursively.
	h_ast_node_statement stmt;
	get_node( stmt, get_child_ast_node( it, 2 ) );

	return_node( ast_node_labeled_statement::create( lbl, stmt ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_expression_statement ){
	h_ast_node_expression expr;
	get_child_ast_node( expr, it, 0 );
	return_node(
		ast_node_expression_statement::create( expr )
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_declaration_statement ){
	h_ast_node_declaration decl;
	get_child_ast_node( decl, it, 0 );
	return_node(
		ast_node_declaration_statement::create( decl )
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_statement ){
	return_node(
		get_child_ast_node( it, 0 )
		);
}