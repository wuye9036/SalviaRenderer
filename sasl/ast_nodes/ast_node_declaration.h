#ifndef SASL_AST_NODE_DECLARATION_H
#define SASL_AST_NODE_DECLARATION_H

#include "ast_node.h"

DECL_HANDLE( ast_node_type )

DECL_HANDLE( ast_node_declaration );
class ast_node_declaration : public ast_node{
};

DECL_HANDLE( ast_node_block_declaration );
class ast_node_block_declaration: public ast_node_declaration{
public:
	static h_ast_node_block_declaration create(
		const std::vector<h_ast_node_variable_declaration>& decls
		);

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	//others
	const std::vector<h_ast_node_variable_declaration>& get_declarations() const;

private:
	ast_node_block_declaration(
		const std::vector<h_ast_node_variable_declaration>& decls
		);
	std::vector<h_ast_node_variable_declaration> decls_;

};

DECL_HANDLE( ast_node_variable_declaration );
class ast_node_variable_declaration : public ast_node_declaration{
public:
	static h_ast_node_variable_declaration create(
		h_ast_node_type var_type,
		h_ast_node_identifier identifier,
		h_ast_node_expression initializer
		);

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	//others
	h_ast_node_type get_type() const;
	h_ast_node_identifier get_identifier() const;
	h_ast_node_expression get_initializer() const;

private:
	ast_node_variable_declaration(
		h_ast_node_type var_type,
		h_ast_node_identifier identifier,
		h_ast_node_expression initializer
		);
	h_ast_node_type var_type_;
	h_ast_node_identifier identifier_;
	h_ast_node_expression initializer_;
};

DECL_HANDLE( ast_node_initialized_declarator );
class ast_node_initialized_declarator: public ast_node{
public:
	static h_ast_node_initialized_declarator create( h_ast_node_identifier ident, h_ast_node_expression init_expr );

	h_ast_node_identifier get_identifier() const;
	h_ast_node_expression get_initializer() const;

	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();
private:
	ast_node_initialized_declarator( h_ast_node_identifier ident, h_ast_node_expression init_expr );
	h_ast_node_identifier ident_;
	h_ast_node_expression initializer_;
};
#endif