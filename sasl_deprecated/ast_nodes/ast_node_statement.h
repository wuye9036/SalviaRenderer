#ifndef SPIRIT_SASL_AST_NODE_STATEMENT_H_B530FB44_F5C8_431F_A250_E426F791B7CF
#define SPIRIT_SASL_AST_NODE_STATEMENT_H_B530FB44_F5C8_431F_A250_E426F791B7CF

#include "ast_node.h"
#include "../enums/control_statements.h"
#include "../enums/labels.h"

DECL_HANDLE( ast_node_expression );
DECL_HANDLE( ast_node_declaration );

DECL_HANDLE( ast_node_statement );
class ast_node_statement: public ast_node{
};

DECL_HANDLE( ast_node_compound_statement );
class ast_node_compound_statement: public ast_node_statement{
public:
	static h_ast_node_compound_statement create( const std::vector<h_ast_node_statement>& stmts );
	const std::vector<h_ast_node_statement>& get_statements() const;

	void visit(ast_visitor* visitor);
	std::string get_typename() const;

private:
	ast_node_compound_statement( const std::vector<h_ast_node_statement>& stmts );
	std::vector<h_ast_node_statement> stmts_;
};

DECL_HANDLE( ast_node_declaration_statement );
class ast_node_declaration_statement: public ast_node_statement{
public:
	static h_ast_node_declaration_statement create(h_ast_node_declaration decl);

	void visit(ast_visitor* visitor);
	std::string get_typename() const;

	h_ast_node_declaration get_declaration() const;

private:
	ast_node_declaration_statement(h_ast_node_declaration decl);
	h_ast_node_declaration decl_;
};

DECL_HANDLE( ast_node_expression_statement );
class ast_node_expression_statement: public ast_node_statement{
public:
	static h_ast_node_expression_statement create(h_ast_node_expression expr);
	
	void visit(ast_visitor* visitor);
	std::string get_typename() const;

	h_ast_node_expression get_expression() const;
private:
	ast_node_expression_statement(h_ast_node_expression expr);
	h_ast_node_expression expr_;
};

DECL_HANDLE( ast_node_if_statement );
class ast_node_if_statement: public ast_node_statement{
public:
	static h_ast_node_if_statement create(h_ast_node_expression cond_expr, h_ast_node_statement if_stmt, h_ast_node_statement else_stmt);

	h_ast_node_expression get_condition_expression();
	h_ast_node_statement get_if_statement();
	h_ast_node_statement get_else_statement();

	void visit(ast_visitor* visitor);
	std::string get_typename() const;

private:
	ast_node_if_statement(h_ast_node_expression cond_expr, h_ast_node_statement if_stmt, h_ast_node_statement else_stmt);
	h_ast_node_expression cond_expr_;
	h_ast_node_statement if_stmt_;
	h_ast_node_statement else_stmt_;
};

DECL_HANDLE( ast_node_switch_statement );
class ast_node_switch_statement: public ast_node_statement{
public:
	typedef std::vector<h_ast_node_statement> stmt_list_t;

	static h_ast_node_switch_statement create( h_ast_node_expression cond_expr, const std::vector<h_ast_node_statement>& stmts );

	void visit(ast_visitor* visitor);
	std::string get_typename() const;

	h_ast_node_expression get_condition_expression() const;
	const std::vector<h_ast_node_statement>& get_statements() const;
private:
	ast_node_switch_statement( h_ast_node_expression cond_expr, const std::vector<h_ast_node_statement>& stmts );
	h_ast_node_expression cond_expr_;
	std::vector<h_ast_node_statement> stmts_;
};

DECL_HANDLE( ast_node_while_statement );
class ast_node_while_statement: public ast_node_statement{
public:
	static h_ast_node_while_statement create(h_ast_node_expression cond_expr, h_ast_node_statement stmt);

	void visit(ast_visitor* visitor);
	std::string get_typename() const;

	h_ast_node_expression get_condition_expression() const;
	h_ast_node_statement get_statement() const;
private:
	ast_node_while_statement(h_ast_node_expression cond_expr, h_ast_node_statement stmt);

	h_ast_node_expression cond_expr_;
	h_ast_node_statement stmt_;
};

DECL_HANDLE( ast_node_do_while_statement );
class ast_node_do_while_statement: public ast_node_statement{
public:
	static h_ast_node_do_while_statement create( h_ast_node_statement stmt, h_ast_node_expression cond_expr );

	void visit(ast_visitor* visitor);
	std::string get_typename() const;

	h_ast_node_statement get_statement() const;
	h_ast_node_expression get_condition_expression() const;
	
private:
	ast_node_do_while_statement( h_ast_node_statement stmt, h_ast_node_expression cond_expr );

	h_ast_node_statement stmt_;
	h_ast_node_expression cond_expr_;
};

DECL_HANDLE( ast_node_for_statement );
class ast_node_for_statement: public ast_node_statement{
public:
	static h_ast_node_for_statement create( 
		h_ast_node_statement init_stmt, 
		h_ast_node_expression cond_expr, 
		h_ast_node_expression loop_expr,
		h_ast_node_statement body
		);

	void visit(ast_visitor* visitor);
	std::string get_typename() const;

	h_ast_node_statement get_initiaiize_statement() const;
	h_ast_node_expression get_condition_expression() const;
	h_ast_node_expression get_loop_expression() const;
	h_ast_node_statement get_statement() const;

private:
	ast_node_for_statement( 
		h_ast_node_statement init_stmt, 
		h_ast_node_expression cond_expr, 
		h_ast_node_expression loop_expr,
		h_ast_node_statement body
		);

	h_ast_node_statement init_stmt_;
	h_ast_node_expression cond_expr_;
	h_ast_node_expression loop_expr_;
	h_ast_node_statement body_;
};

DECL_HANDLE( ast_node_control_statement );
class ast_node_control_statement: public ast_node_statement{
public:
	static h_ast_node_control_statement create(const std::string& stmt_str, h_ast_node_expression expr = h_ast_node_expression());
	static h_ast_node_control_statement create(const control_statements& stmt, h_ast_node_expression expr = h_ast_node_expression());
	
	void visit(ast_visitor* visitor);
	std::string get_typename() const;

	control_statements get_control_statement() const;
	h_ast_node_expression get_expression() const;

private:
	ast_node_control_statement(control_statements ctrl_stmt, h_ast_node_expression expr);
	control_statements ctrl_stmt_;
	h_ast_node_expression expr_;
};

DECL_HANDLE( ast_node_label )
class ast_node_label: public ast_node{
public:
	static h_ast_node_label create( labels lbl, h_ast_node_identifier ident );
	static h_ast_node_label create( labels lbl, h_ast_node_expression expr );
	static h_ast_node_label create( labels lbl );

	void visit( ast_visitor* visitor );
	std::string get_typename() const;

	labels get_label_type() const;
	h_ast_node_identifier get_label() const;
	h_ast_node_expression get_expression() const;

private:
	ast_node_label( labels lbl );
	ast_node_label( labels lbl, h_ast_node_identifier ident );
	ast_node_label( labels lbl, h_ast_node_expression expr );

	labels lbltype_;
	h_ast_node_identifier ident_;
	h_ast_node_expression expr_;
};

DECL_HANDLE( ast_node_labeled_statement )
class ast_node_labeled_statement: public ast_node_statement{
public:
	static h_ast_node_labeled_statement create( h_ast_node_label lbl, h_ast_node_statement stmt );

	void visit( ast_visitor* visitor );
	std::string get_typename() const;

	h_ast_node_label get_label() const;
	h_ast_node_statement get_statement() const;

private:
	ast_node_labeled_statement( h_ast_node_label lbl, h_ast_node_statement stmt );
	h_ast_node_label lbl_;
	h_ast_node_statement stmt_;
};

#endif //SPIRIT_SASL_AST_NODE_STATEMENT_H_B530FB44_F5C8_431F_A250_E426F791B7CF
