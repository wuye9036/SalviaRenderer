//ast node type mush have some members:
//  get_hash();
//	equal();

//execute bytecode of Softart Shader Middle Language ( is compatiable with VMIL which designed and implements by vczh )
class bytecode_executor{
};

//JIT Bytecode Executor
class jit_executor: public bytecode_executor{
};

//Virtual Machine Executor
class virtual_machine_executor: bytecode_executor{
};

#define DECLARE_ENTER_VISITOR( node_name ) \
	virtual void enter_node ( WEAK_HANDLE_OF( BOOST_PP_CAT( ast_node_, node_name ) ) node )
#define DECLARE_LEAVE_VISITOR( node_name ) \
	virtual void leave_node ( WEAK_HANDLE_OF( BOOST_PP_CAT( ast_node_, node_name ) ) node )

#define DECLARE_DEFAULT_VISITOR( node_name ) \
	DECLARE_ENTER_VISITOR( node_name ){} \
	DECLARE_LEAVE_VISITOR( node_name ){}

#define DECLARE_DEFAULT_VISITOR_ITEM( r, data, node_name ) DECLARE_DEFAULT_VISITOR( node_name )
#define DECLARE_DEFAULT_VISITORS( node_seq ) BOOST_PP_SEQ_FOR_EACH( DECLARE_DEFAULT_VISITOR_ITEM, 0, node_seq )

class ast_visitor{
public:
	DECLARE_DEFAULT_VISITORS(
		( scalar_type ) ( vector_type ) ( matrix_type )
		( variable_declaration ) 
		( literal_expression ) ( variable_expression ) ( cast_expression ) 
		( unary_expression ) ( binary_expression ) ( condition_expression ) ( expression_list )
		( call_expression ) ( index_expression ) ( member_expression )
		( declaration_statement ) ( expression_statement )
		( if_statement ) ( switch_statement ) 
		( while_statement ) ( do_while_statement )
		( for_statement )
		( control_statement )
		( compound_statement )
		( labeled_statement )
		( function ) ( array ) ( struct )
	);

	virtual void enter_scope() = 0;
	virtual void leave_scope() = 0;
};

class semantic_constraint{
	bool rvalue();
	bool lvalue();
	bool constant_expression();
};

//semantic check will finish 2 tasks:
//	1. build symbol table,
//	2. compilation-time constant expression check. 
//	3. invoke parameter and member check.
class scope_builder: public ast_visitor{
	void enter_function( h_ast_node_function_definition func_node ){
		//can not inline function in other function.
		enter_scope();
	}
	void leave_function( h_ast_node_function_definition func_node ){
		leave_scope();
	}

	void enter_structure( h_ast_node_structure_type struct_type_node ){
		enter_scope();
	}

	void leave_structure( h_ast_node_structure_type struct_type_node ){
		leave_scope();
	}

	void add_variable_declaration( h_ast_node_variable_declaration decl ){
		bool is_valid_type = check_type( param_decl_node->get_type() );
		if( !is_valid_type ){
			current_scope_->report_error( error_codes::invalid_type, param_decl_node );
		}

		string parameter_name;
		h_ast_node_identifier param_name_node = param_decl_node->get_identifier();
		if( param_name_node ){
			parameter_name = param_name_node->get_ident();
		} else {
			parameter_name = current_scope_->next_auto_name()
		}

		current_scope_->push_back_symbol( parameter_name, param_decl_node );
	}

	void enter_parameter_declaration( h_ast_node_variable_declaration param_decl_node ){
		add_variable_declaration( param_decl_node );
	}

	void leave_parameter_declaration( h_ast_node_declaration param_decl_node ){
	}

	void enter_variable_declaration( h_ast_node_variable_declaration decl_node ){
		add_variable_declaration( decl_node );
	}

	void leave_variable_declaration();

	void enter_typedef_declaration();
	void leave_typedef_declaration();

	void enter_if_statement( h_ast_node_if_statement stmt ){
	}

	void leave_if_statement( h_ast_node_if_statement stmt ){
	}

	void enter_if_branch_statement( h_ast_node_if_statement stmt ){
		enter_scope();
	}

	void leave_if_branch_statement( h_ast_node_if_statement stmt ){
		leave_scope();
	}

	void enter_else_branch_statement( h_ast_node_if_statement stmt ){
		enter_scope();
	}

	void leave_else_branch_statement( h_ast_node_if_statement stmt ){
	}

	void enter_binary_expression(){
	}

	void enter_unary_expression(){
	}

	void enter_call_expression( h_ast_node_call_expression expr ){
		// function type deducer
		// parameters deducer
		// push result type to deducer
	}

	void leave_call_expression( h_ast_node_call_expression expr ){
	}

	void enter_member_expression( h_ast_node_member_expression expr ){
		// pop object type
		// check member type
		// push member type to type deducer
	}

	void enter_labeled_statement( h_ast_node_labeled_statement stmt ){
		// case label must in a switch scope
		// default label must in a switch scope
	}

	void leave_labeled_statement( h_ast_node_labeled_statement stmt ){
		//process variable declaration through
	}

	void enter_control_statement( h_ast_node_control_statement stmt ){
		//
		if ( stmt->get_control_statements() == control_statements::_break ){
			if( breakable_count_ <= 0 ){
				current_scope_->error_report( semantic_errors::break_out_of_cycle_scope, stmt );
			}
		}

		// do continue

		// do return
		// get function return type
		// match type of return expression and function
	}

	void leave_control_statement( h_ast_node_control_statement stmt ){
	}

private:
	size_t breakable_count_;
	size_t continuable_count_;
	size_t caseable_count_;

	h_ast_node_type current_function_type;

	void enter_scope();
	void leave_scope();

	h_scope root_;
	weak_ptr<scope> current_scope_;
	vector< semantic_constraints > constaints_;
};

// interpret codes.
class interpreter{
public:
	typedef pair< size_t, size_t > iterator;
	void interpret()

	void interpret( bytecode_executor& executor );
private:
	void build_scopes(){
		
	}
	scope global_scope_;
	scope* current_scope_;
	typedef h_ast_node iterator;
	interpret_iterator current_iterator;
};

class scope{
public:
	h_scope parent();
	h_scope prev_sibling();
	h_scope next_sibling();

private:
	typedef size_t instruction_position_t;
	typedef size_t address_t;

	label_table labels_;
	symbol_table symbols_;

	vector< scope > subscopes_;

	//get position of it's siblings
	weak< h_scope >::type parent_;
	size_t index_;
};

DECL_HANDLE( symbol_table )
class symbol_table{
public:
	symbol find_symbol( string str );
	symbol push_back( const symbol& sym);
private:
	unordered_map< string, symbol > symbols_;
};

class symbol{
private:
	string name_;
	
	// location in stack
	size_t offset_;
	size_t size_;

	// interact with c function.
	intptr_t c_interface;

	ast_node_type type_;
	bool is_typedef_;
};

class label{
	h_ast_node_label label_node;
	interpret_iterator it;
};

class label_table{
	label get_identifier_label( string label_name );
};

class interpret_context{
	vector<>
};