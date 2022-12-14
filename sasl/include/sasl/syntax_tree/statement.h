#ifndef SASL_SYNTAX_TREE_STATEMENT_H
#define SASL_SYNTAX_TREE_STATEMENT_H

#include <sasl/syntax_tree/syntax_tree_fwd.h>
#include <sasl/syntax_tree/node.h>
#include <sasl/enums/jump_mode.h>

#include <vector>
#include <memory>

namespace sasl::syntax_tree {

using sasl::common::token_t;

struct compound_statement;
struct declaration;
struct expression;
struct identifier;
struct label;

struct statement: public node{
	statement( node_ids type_id, std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
};

struct labeled_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<struct label> pop_label();
	void push_label( std::shared_ptr<label> lbl );

	std::shared_ptr<statement> stmt;
	std::vector<std::shared_ptr<struct label> > labels;

private:
	labeled_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	labeled_statement& operator = ( const labeled_statement& );
	labeled_statement( const labeled_statement& );
};

struct declaration_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::vector< std::shared_ptr<declaration> > decls;
private:
	declaration_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	declaration_statement& operator = ( const declaration_statement& );
	declaration_statement( const declaration_statement& );
};

struct if_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr< expression > cond;
	std::shared_ptr< statement > yes_stmt, no_stmt;
private:
	if_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	if_statement& operator = ( const if_statement& );
	if_statement( const if_statement& );
};

struct while_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<expression> cond;
	std::shared_ptr<statement> body;
private:
	while_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	while_statement& operator = ( const while_statement& );
	while_statement( const while_statement& );
};

struct dowhile_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<statement> body;
	std::shared_ptr<expression> cond;
private:
	dowhile_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	dowhile_statement& operator = (const dowhile_statement& );
	dowhile_statement( const dowhile_statement& );
};

struct for_statement: public statement
{
public:
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<statement> init;
	std::shared_ptr<expression> cond;
	std::shared_ptr<expression> iter;
	std::shared_ptr<compound_statement> body;
private:
	for_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	for_statement( const for_statement& rhs);
	for_statement& operator = ( const for_statement& rhs );
};
struct label: public node{
	label( node_ids type_id, std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
};

struct case_label : public label{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	// if expr is null pointer, it means default.
	std::shared_ptr<expression> expr;
private:
	case_label( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	case_label& operator = ( const case_label& );
	case_label( const case_label& );
};

struct ident_label: public label{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<token_t> label_tok;
private:
	ident_label( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	ident_label& operator = ( const ident_label& );
	ident_label( const ident_label& );
};

struct switch_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<expression> cond;
	std::shared_ptr<compound_statement> stmts;
private:
	switch_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	switch_statement& operator = ( const switch_statement& );
	switch_statement( const switch_statement& );
};

struct compound_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::vector< std::shared_ptr<statement> > stmts;
private:
	compound_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	compound_statement& operator = ( const compound_statement& );
	compound_statement( const compound_statement& );
};

struct expression_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<expression> expr;
private:
	expression_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	expression_statement& operator = ( const expression_statement& );
	expression_statement( const expression_statement& );
};

struct jump_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	jump_mode code;
	std::shared_ptr<expression> jump_expr; // for return only
private:
	jump_statement( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	jump_statement& operator = ( const jump_statement& );
	jump_statement( const jump_statement& );
};

}

#endif
