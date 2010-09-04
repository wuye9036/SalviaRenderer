#ifndef SASL_SYNTAX_TREE_STATEMENT_H
#define SASL_SYNTAX_TREE_STATEMENT_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/jump_mode.h>
#include <boost/shared_ptr.hpp>
#include <vector>

BEGIN_NS_SASL_SYNTAX_TREE();

using sasl::common::token_attr;

struct compound_statement;
struct declaration;
struct expression;
struct identifier;
struct label;

struct statement: public node{
	statement( syntax_node_types type_id, boost::shared_ptr<token_attr> tok );
	boost::shared_ptr<struct label> label() const;
	template <typename T> void label( boost::shared_ptr<T> lbl ){
		this->lbl = boost::shared_polymorphic_cast<struct label>(lbl);
	}
private:
	boost::shared_ptr<struct label> lbl;
};

struct declaration_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	void accept( syntax_tree_visitor* v );

	boost::shared_ptr<declaration> decl;
private:
	declaration_statement( boost::shared_ptr<token_attr> tok );
	declaration_statement& operator = ( const declaration_statement& );
	declaration_statement( const declaration_statement& );
};

struct if_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	void accept( syntax_tree_visitor* v );
	
	boost::shared_ptr< expression > cond;
	boost::shared_ptr< statement > yes_stmt, no_stmt;
private:
	if_statement( boost::shared_ptr<token_attr> tok );
	if_statement& operator = ( const if_statement& );
	if_statement( const if_statement& );
};

struct while_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	void accept( syntax_tree_visitor* v );

	boost::shared_ptr<expression> cond;
	boost::shared_ptr<statement> body;
private:
	while_statement( boost::shared_ptr<token_attr> tok );
	while_statement& operator = ( const while_statement& );
	while_statement( const while_statement& );
};

struct dowhile_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	void accept( syntax_tree_visitor* v );

	boost::shared_ptr<statement> body;
	boost::shared_ptr<expression> cond;
private:
	dowhile_statement( boost::shared_ptr<token_attr> tok );
	dowhile_statement& operator = (const dowhile_statement& );
	dowhile_statement( const dowhile_statement& );
};

//struct label: public node{
//};
//
//struct case_label : public label{
//	// if expr is null pointer, it means default.
//	boost::shared_ptr<expression> expr;
//	size_t entry_index;
//};
//
//struct jump_label: public label{
//	boost::shared_ptr<token_attr> label_tok;
//};

struct switch_statement{
	boost::shared_ptr<expression> cond;
	boost::shared_ptr<compound_statement> stmts;
};

struct compound_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	void accept( syntax_tree_visitor* v );

	std::vector< boost::shared_ptr<statement> > stmts;
private:
	compound_statement( boost::shared_ptr<token_attr> tok );
	compound_statement& operator = ( const compound_statement& );
	compound_statement( const compound_statement& );
};

struct expression_statement: public statement{
	SASL_SYNTAX_NODE_CREATORS();

	void accept( syntax_tree_visitor* v );

	boost::shared_ptr<expression> expr;
private:
	expression_statement( boost::shared_ptr<token_attr> tok );
	expression_statement& operator = ( const expression_statement& );
	expression_statement( const expression_statement& );
};

struct jump_statement{
	jump_mode code;
	boost::shared_ptr<expression> jump_expr;
};

END_NS_SASL_SYNTAX_TREE()

#endif