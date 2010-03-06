#ifndef SASL_AST_NODE_EXPRESSION_H
#define SASL_AST_NODE_EXPRESSION_H

#include "ast_node.h"
#include "../enums/operators.h"

DECL_HANDLE( ast_node_type );

DECL_HANDLE( ast_node_variable_expression );
class ast_node_variable_expression: public ast_node_expression{
public:
	static h_ast_node_variable_expression create( h_ast_node_identifier ident );

	//inherited
	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	h_ast_node_identifier get_identifier() const;

private:
	ast_node_variable_expression( h_ast_node_identifier ident );
	h_ast_node_identifier ident_;
};

DECL_HANDLE( ast_node_cast_expression );
class ast_node_cast_expression: public ast_node_expression{
public:
	static h_ast_node_cast_expression create( h_ast_node_type type_spec, h_ast_node_expression expr );

	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
private:
	ast_node_cast_expression( h_ast_node_type type_spec, h_ast_node_expression expr );
	
	h_ast_node_type type_spec_;
	h_ast_node_expression expr_;
};

DECL_HANDLE( ast_node_unary_expression );
class ast_node_unary_expression: public ast_node_expression{
public:
	static h_ast_node_unary_expression create( operators op, h_ast_node_expression expr );

	//inherited
	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	h_ast_node_expression get_expression() const;
	operators get_op() const;

private:
	ast_node_unary_expression( operators op, h_ast_node_expression expr );

	h_ast_node_expression expr_;
	operators op_;
};

DECL_HANDLE( ast_node_call_expression );
class ast_node_call_expression: public ast_node_expression{
public:
	static h_ast_node_call_expression create( h_ast_node_expression function_expr, const std::vector<h_ast_node_expression>& parameter_exprs );

	//inherited
	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();
	
	h_ast_node_expression get_function_expression() const;
	const std::vector<h_ast_node_expression>& get_parameter_expressions() const;
private:
	ast_node_call_expression( h_ast_node_expression function_expr, const std::vector<h_ast_node_expression>& parameter_exprs );
	h_ast_node_expression function_expr_;
	std::vector<h_ast_node_expression> param_exprs_;
};

DECL_HANDLE( ast_node_index_expression );
class ast_node_index_expression: public ast_node_expression{
public:
	static h_ast_node_index_expression create( h_ast_node_expression array_expr, h_ast_node_expression index_expr );

	//inherited
	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	h_ast_node_expression get_array_expression() const;
	h_ast_node_expression get_index_expression() const;

private:
	ast_node_index_expression( h_ast_node_expression array_expr, h_ast_node_expression index_expr );
	
	h_ast_node_expression array_expr_;
	h_ast_node_expression index_expr_;
};

DECL_HANDLE( ast_node_member_expression );
class ast_node_member_expression: public ast_node_expression{
public:
	static h_ast_node_member_expression create( h_ast_node_expression obj_expr, h_ast_node_identifier ident );
	
	//inherited
	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	h_ast_node_expression get_obj_expression() const;
	h_ast_node_identifier get_member_identifier() const;

private:
	ast_node_member_expression( h_ast_node_expression obj_expr, h_ast_node_identifier member_ident );
	h_ast_node_expression obj_expr_;
	h_ast_node_identifier member_identifier_;
};

DECL_HANDLE( ast_node_binary_expression );
class ast_node_binary_expression: public ast_node_expression{
public:
	static h_ast_node_binary_expression create( h_ast_node_expression lexpr, operators op, h_ast_node_expression rexpr );

	//inherited
	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	h_ast_node_expression get_left_expressions() const;
	h_ast_node_expression get_right_expression() const;
	operators get_op() const;

private:
	ast_node_binary_expression( h_ast_node_expression lexpr, operators op, h_ast_node_expression rexpr );
	h_ast_node_expression left_expr_;
	h_ast_node_expression right_expr_;
	operators op_;
};

DECL_HANDLE( ast_node_condition_expression );
class ast_node_condition_expression: public ast_node_expression{
public:
	static h_ast_node_condition_expression create( h_ast_node_expression cond_expr, h_ast_node_expression true_expr, h_ast_node_expression false_expr );

	//inherited
	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	h_ast_node_expression get_condition_expression() const;
	h_ast_node_expression get_true_expression() const;
	h_ast_node_expression get_false_expression() const;
private:
	ast_node_condition_expression( h_ast_node_expression cond_expr, h_ast_node_expression true_expr, h_ast_node_expression false_expr );
	h_ast_node_expression cond_expr_;
	h_ast_node_expression true_expr_;
	h_ast_node_expression false_expr_;
};

DECL_HANDLE( ast_node_expression_list )
class ast_node_expression_list: public ast_node_expression{
public:
	static h_ast_node_expression_list create( const std::vector<h_ast_node_expression>& exprs );

	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);

	const std::vector<h_ast_node_expression>& get_expressions() const;

private:
	ast_node_expression_list( const std::vector<h_ast_node_expression>& exprs );
	std::vector<h_ast_node_expression> exprs_;
};
#endif

