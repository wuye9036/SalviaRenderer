#ifndef SASL_SYNTAX_TREE_STATEMENT_H
#define SASL_SYNTAX_TREE_STATEMENT_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/jump_mode.h>
#include <boost/shared_ptr.hpp>

BEGIN_NS_SASL_SYNTAX_TREE()

using sasl::common::token_attr;

struct statement: public node{
	statement( syntax_node_types type_id, boost::shared_ptr<token_attr> tok );
};

struct declaration_statement: public statement{
	declaration_statement( boost::shared_ptr<token_attr> tok );
	void accept( syntax_tree_visitor* v );
	declaration decl;
};

struct if_statement: public statement{
	boost::shared_ptr< expression > cond;
	boost::shared_ptr< statement > 
		yes_stmt, no_stmt;
};

struct while_statement{
	boost::shared_ptr<expression> cond;
	boost::shared_ptr<statement> body;
};

struct dowhile_statement{
	boost::shared_ptr<statement> body;
	boost::shared_ptr<expression> cond;
};

struct case_label{
	boost::shared_ptr<expression> expr;
	size_t entry_index;
};

struct switch_statement{
	boost::shared_ptr<expression> cond;
	std::vector< boost::shared_ptr<statement> > stmts;
	std::vector< boost::shared_ptr<case_label> > case_labels;
};

struct compound_statement{
	std::vector< boost::shared_ptr<statement> > stmts;
	std::vector< boost::shared_ptr<identifier> > jump_labels;
};

struct expression_statement{
	boost::shared_ptr<expression> expr;
};

struct jump_statement{
	jump_mode code;
	boost::shared_ptr<expression> jump_expr;
};

END_NS_SASL_SYNTAX_TREE()

#endif