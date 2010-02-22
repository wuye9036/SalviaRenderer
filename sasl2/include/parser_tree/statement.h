#ifndef SASL_PARSER_TREE_STATEMENT_H
#define SASL_PARSER_TREE_STATEMENT_H

#include "parser_tree_forward.h"
#include <boost/variant.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/tuple.hpp>
#include <vector>
BEGIN_NS_SASL_PARSER_TREE()

struct basic_declaration;
struct function_definition;
typedef boost::variant< 
	boost::recursive_wrapper< function_definition >, 
	boost::recursive_wrapper< basic_declaration >
> declaration;
typedef declaration declaration_statement;

struct if_statement;
struct while_statement;
struct dowhile_statement;
struct for_statement;
struct switch_statement;
struct expression_statement;
struct compound_statement;
struct jump_statement;
struct return_statement;
struct labeled_statement;

typedef boost::variant<
	boost::recursive_wrapper<jump_statement>,
	boost::recursive_wrapper<return_statement>
> flowcontrol_statement;

typedef boost::variant<
	boost::recursive_wrapper<declaration_statement>,
	boost::recursive_wrapper<if_statement>,
	boost::recursive_wrapper<while_statement>,
	boost::recursive_wrapper<dowhile_statement>,
	boost::recursive_wrapper<for_statement>,
	boost::recursive_wrapper<switch_statement>,
	boost::recursive_wrapper<expression_statement>,
	boost::recursive_wrapper<compound_statement>,
	boost::recursive_wrapper<flowcontrol_statement>,
	boost::recursive_wrapper<labeled_statement>
> statement;

struct if_statement{
	typedef boost::fusion::vector<token_attr, statement> else_part_t;
	typedef boost::optional<else_part_t> optional_else_t;

	token_attr if_keyword;
	token_attr lparen;
	expression cond;
	token_attr rparen;
	statement stmt;
	optional_else_t else_part;
};

typedef boost::variant<
	boost::recursive_wrapper<declaration_statement>,
	boost::recursive_wrapper<expression_statement>
> for_initializer;

struct for_loop_header{
	typedef boost::optional<expression> condition_t;
	typedef boost::optional<expression> step_t;

	token_attr lparen;
	for_initializer for_init;
	condition_t cond;
	token_attr semicolon;
	step_t step;
	token_attr rparen;
};

struct for_statement{
	token_attr for_keyword;
	for_loop_header looper;
	statement stmt;
};

struct while_statement{
	token_attr while_keyword;
	token_attr lparen;
	expression cond;
	token_attr rparen;
	statement stmt;
};

struct dowhile_statement{
	token_attr do_keyword;
	statement stmt;
	token_attr while_keyword;
	token_attr lparen;
	expression cond;
	token_attr rparen;
};

struct expression_statement{
	expression expr;
	token_attr semicolon;
};

struct compound_statement{
	typedef std::vector< statement > stmts_t;
	token_attr lbrace;
	stmts_t stmts;
	token_attr rbrace;
};

struct jump_statement{
	token_attr break_or_continue_keyword;
	token_attr semicolon;
};

struct return_statement{
	typedef boost::optional<expression> return_expression_t;
	token_attr return_keyword;
	return_expression_t retexpr;
	token_attr semicolon;
};

struct labeled_statement{
	typedef boost::fusion::vector<token_attr, expression> case_label_t;
	typedef boost::variant<token_attr, case_label_t> label_t;
	label_t label;
	token_attr colon;
	statement stmt;
};

struct switch_statement{
	typedef std::vector<statement> stmts_t;
	token_attr switch_keyword;
	token_attr lparen;
	expression expr;
	token_attr rparen;
	token_attr lbrace;
	stmts_t stmts;
	token_attr rbrace;
};

END_NS_SASL_PARSER_TREE()

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::if_statement,
						  (token_attr, if_keyword)
						  (token_attr, lparen)
						  (sasl::parser_tree::expression, cond)
						  (token_attr, rparen)
						  (sasl::parser_tree::statement, stmt)
						  (sasl::parser_tree::if_statement::optional_else_t, else_part)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::for_loop_header,
						  (token_attr, lparen)
						  (sasl::parser_tree::for_initializer, for_init)
						  (sasl::parser_tree::for_loop_header::condition_t, cond)
						  (token_attr, semicolon)
						  (sasl::parser_tree::for_loop_header::step_t, step)
						  (token_attr, rparen)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::for_statement,
						  (token_attr, for_keyword)
						  (sasl::parser_tree::for_loop_header, looper)
						  (sasl::parser_tree::statement, stmt)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::while_statement,
						  (token_attr, while_keyword)
						  (token_attr, lparen)
						  (sasl::parser_tree::expression, cond)
						  (token_attr, rparen)
						  (sasl::parser_tree::statement, stmt)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::dowhile_statement,
						  (token_attr, do_keyword)
						  (sasl::parser_tree::statement, stmt)
						  (token_attr, while_keyword)
						  (token_attr, lparen)
						  (sasl::parser_tree::expression, cond)
						  (token_attr, rparen)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::expression_statement,
						  (sasl::parser_tree::expression, expr)
						  (token_attr, semicolon)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::compound_statement,
						  (token_attr, lbrace)
						  (sasl::parser_tree::compound_statement::stmts_t, stmts)
						  (token_attr, rbrace)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::jump_statement,
						  (token_attr, break_or_continue_keyword)
						  (token_attr, semicolon)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::return_statement,
						  (token_attr, return_keyword)
						  (sasl::parser_tree::return_statement::return_expression_t, retexpr)
						  (token_attr, semicolon)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::labeled_statement,
						  (sasl::parser_tree::labeled_statement::label_t, label)
						  (token_attr, colon)
						  (sasl::parser_tree::statement, stmt)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::switch_statement,
						  (token_attr, switch_keyword)
						  (token_attr, lparen)
						  (sasl::parser_tree::expression, expr)
						  (token_attr, rparen)
						  (token_attr, lbrace)
						  (sasl::parser_tree::switch_statement::stmts_t, stmts)
						  (token_attr, rbrace)
						  );
#endif