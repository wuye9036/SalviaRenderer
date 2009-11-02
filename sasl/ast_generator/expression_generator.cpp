DEFINE_TREE_NODE_PROCESSOR( r_bool_expr ){
	typename const tree_node_t::parse_node_t& value = it->value;
	string bool_literal(value.begin(), value.end());

	return_node( ast_node_literal_expression::create( bool_literal, "", literal_types::boolean ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_variable_expr ){
	typename const tree_node_t::children_t& children = it->children;
	h_ast_node_identifier ident;
	get_node( 
		ident, process_rule_node( iter_at(children, 0) ) 
		);

	return_node(
		ast_node_variable_expression::create( ident )
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_expr_list ){
	typename const tree_node_t::children_t& children = it->children;
	
	vector<h_ast_node_expression> exprs;
	for(size_t i_child = 0; i_child < children.size(); ++i_child ){
		h_ast_node_expression cur_expr;
		get_node( cur_expr, process_rule_node( iter_at( children, i_child ) ) );
		exprs.push_back( cur_expr );
	}

	return_node(
		ast_node_expression_list::create( exprs )
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_string_expr ){
	typename const tree_node_t::parse_node_t& value = it->value;
	return_node(
		ast_node_literal_expression::create( 
			string( value.begin(), value.end() ),
			string(),
			literal_types::string
			)
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_int_expr ){
	typename const tree_node_t::children_t& children = it->children;
	string suffix;

	if( children.size() > 1 ){
		suffix = string( value_at( children, 1 ).begin(), value_at( children, 1 ).end() );
	}

	return_node(
		ast_node_literal_expression::create( 
			string(value_at(children, 0).begin(), value_at(children, 0).end()) ,
			suffix,
			literal_types::integer
			)
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_real_expr ){
	const children_t& children = it->children;
	string suffix;

	if( children.size() > 1 ){
		suffix = get_child_string(it, 1);
	}

	return_node(
		ast_node_literal_expression::create( 
			get_child_string(it, 0) ,
			suffix,
			literal_types::real
			)
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_char_expr ){
	typename const tree_node_t::parse_node_t& value = it->value;
	return_node(
		ast_node_literal_expression::create( 
			string( value.begin(), value.begin()+1 ),
			string(),
			literal_types::string
			)
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_literal_expr ){
	typename const tree_node_t::children_t& children = it->children;
	return_node(
		process_rule_node( iter_at(children, 0) )
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_primary_expr ){
	typename const tree_node_t::children_t& children = it->children;
	return_node(
		process_rule_node( iter_at(children, 0) )
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_postfix_expr ){
	typename const tree_node_t::children_t& children = it->children;
	
	h_ast_node_expression current_expr;
	get_node( current_expr, process_rule_node( iter_at(children, 0) ) );

	// if it has multiple expressions, 
	// left combination will be used for taking expression list to a expression tree.
	for( size_t i_expr = 1; i_expr < children.size(); ++i_expr ){
		TreeIteratorT child_it = iter_at( children, i_expr );
		size_t postfix_parser_id = value_at(children, i_expr).id().to_long();
		
		h_ast_node_expression index_expr;
		h_ast_node_identifier ident;
		vector<h_ast_node_expression> exprs;
		handle_type_of< ast_node_value_expression<string> >::result op_str_expr;
		::operators op( ::operators::postfix_incr );
 
		switch( postfix_parser_id ){
			case parser_ids::r_index_expr:
				get_node( index_expr, process_rule_node( child_it ) );
				get_node( current_expr, ast_node_index_expression::create( current_expr, index_expr ) );
				break;
			case parser_ids::r_member_expr:
				get_node( ident, process_rule_node( child_it ) );
				get_node( current_expr, ast_node_member_expression::create( current_expr, ident ) );
				break;
			case parser_ids::r_function_call_expr:
				if( !children.empty() ){
					h_ast_node_expression_list expr_lst;
					get_node( expr_lst, process_rule_node(child_it) );
					exprs = expr_lst->get_expressions();
				}

				get_node( current_expr, ast_node_call_expression::create(current_expr, exprs) );
				break;
			case parser_ids::r_postfix_op_expr:
				get_node( op_str_expr, process_rule_node(child_it) );
				op = postfix_op_symbols::instance().find( op_str_expr->get_value() );
				get_node( current_expr, ast_node_unary_expression::create( op, current_expr ) );
				break;
			default:
				//error
				break;
		}
	}

	return_node( current_expr );
}
DEFINE_TREE_NODE_PROCESSOR( r_index_expr ){
	typename const tree_node_t::children_t& children = it->children;
	return_node( process_rule_node( iter_at(children, 0) ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_function_call_expr ){
	typename const tree_node_t::children_t& children = it->children;
	if( children.empty() ){
		return_node( h_ast_node() );
	} else {
		return_node( process_rule_node( iter_at(children, 0) ) );
	}
}
DEFINE_TREE_NODE_PROCESSOR( r_member_expr ){
	typename const tree_node_t::children_t& children = it->children;
	return_node( process_rule_node( iter_at(children, 0) ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_postfix_op_expr ){
	const value_t& value = it->value;
	return_node( ast_node_value_expression<string>::create( string(value.begin(), value.end()) ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_unary_expr ){
	const children_t& children = it->children;
	if( children.size() == 1 ){
		return_node( process_rule_node( iter_at(children, 0) ) );
	}

	h_ast_node_expression expr;
	get_node( expr, process_rule_node( iter_at(children, 1) ) );

	::operators op = unary_op_symbols::instance().find( get_child_string(it, 0) );
	return_node( ast_node_unary_expression::create( op, expr ) );
}
DEFINE_TREE_NODE_PROCESSOR( r_cast_expr ){
	const children_t& children = it->children;
	
	if( children.size() == 1 ){
		return_node( get_child_ast_node( it, 0 ) );
	}

	h_ast_node_type cast_type;
	get_node( cast_type, get_child_ast_node(it, 0) );

	h_ast_node_expression expr;
	get_node( expr, get_child_ast_node(it, 1) );

	return_node( ast_node_cast_expression::create( cast_type, expr ) );
}

DEFINE_TREE_NODE_PROCESSOR( r_lcombine_binary_expr ){
	const children_t& children = it->children;
	
	h_ast_node_expression left_expr;
	get_node( left_expr, get_child_ast_node( it, 0 ) );

	for( size_t i_child = 1; i_child < children.size(); i_child+=2 /*get a operator and a expression each time*/ ){
		::operators op( binary_op_symbols::instance().find( get_child_string( it, i_child ) ) );
		h_ast_node_expression right_expr;
		get_node( right_expr, get_child_ast_node(it, i_child+1) );

		get_node( left_expr, ast_node_binary_expression::create( left_expr, op, right_expr ) );
	}

	return_node( left_expr );
}

//	NOTE: assignment operators are right-combined.
//		so it will be reserved to a left-combind form.
DEFINE_TREE_NODE_PROCESSOR( r_assignment_expr ){
	children_t reversed_children;
	reverse_container( reversed_children, it->children );
	typename children_t::iterator reversed_it = reversed_children.begin();

	return_node( CALL_PROCESSOR( r_lcombine_binary_expr, reversed_it ) );
}

DEFINE_LCOMB_BINARY_EXPRESSION_PROCESSORS
(
	( r_mul_expr ) ( r_add_expr )
	( r_shift_expr ) ( r_bit_and_expr ) ( r_bit_xor_expr ) ( r_bit_or_expr )
	(r_rel_expr ) ( r_equal_expr ) ( r_or_expr ) ( r_and_expr )
);

DEFINE_TREE_NODE_PROCESSOR( r_cond_expr ){
	const children_t& children = it->children;
	
	if( children.size() == 1 ){
		return_node( get_child_ast_node( it, 0 ) );
	}

	h_ast_node_expression cond_expr;
	h_ast_node_expression true_expr;
	h_ast_node_expression false_expr;

	get_node( cond_expr, get_child_ast_node(it, 0) );
	get_node( true_expr, get_child_ast_node(it, 1) );
	get_node( false_expr, get_child_ast_node(it, 2) );

	return_node(
		ast_node_condition_expression::create( cond_expr, true_expr, false_expr )
		);
}
DEFINE_TREE_NODE_PROCESSOR( r_assignment_rhs_expr ){
	return_node( get_child_ast_node(it, 0) );
}
DEFINE_TREE_NODE_PROCESSOR( r_expression ){
	return_node( get_child_ast_node(it, 0) );
}