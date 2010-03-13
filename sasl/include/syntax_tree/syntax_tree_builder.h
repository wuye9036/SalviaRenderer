#ifndef SASL_SYNTAX_TREE_SYNTAX_TREE_BUILDER_H
#define SASL_SYNTAX_TREE_SYNTAX_TREE_BUILDER_H

#include "syntax_tree_fwd.h"
#include "constant.h"
#include "expression.h"
#include <sasl/include/parser_tree/literal.h>
#include <sasl/include/parser_tree/expression.h>
#include <boost/variant/static_visitor.hpp>

BEGIN_NS_SASL_SYNTAX_TREE()

using sasl::common::token_attr;

class syntax_tree_builder{
public:
	// build terminators
	virtual operators build_operator( const sasl::common::token_attr& v, operators modifier ) = 0;
	virtual boost::shared_ptr<identifier> build_identifier( const sasl::common::token_attr& v) = 0;
	virtual boost::shared_ptr<constant> build_constant( const sasl::common::token_attr& v ) = 0;

	// build expressions
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::unary_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::cast_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::mul_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::add_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::shf_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::rel_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::eql_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::band_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::bxor_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::bor_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::land_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::lor_expression&  v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::rhs_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::assign_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::expression_post& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::pm_expression& v ) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::typecast_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::post_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::cond_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::idx_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::call_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::mem_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::unaried_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::paren_expression& v ) = 0;

	// build declarations
	virtual boost::shared_ptr<declaration> build( const sasl::parser_tree::declaration& v ) = 0;
	virtual boost::shared_ptr<declaration> build( const sasl::parser_tree::variable_declaration& v ) = 0;
	virtual boost::shared_ptr<declaration> build( const sasl::parser_tree::basic_declaration& v ) = 0;
	virtual boost::shared_ptr<declaration> build( const sasl::parser_tree::function_declaration& v ) = 0;
	virtual boost::shared_ptr<declaration> build( const sasl::parser_tree::function_definition& v ) = 0;
	virtual boost::shared_ptr<declaration> build( const sasl::parser_tree::struct_declaration& v) = 0;
	virtual boost::shared_ptr<declaration> build( const sasl::parser_tree::typedef_declaration& v) = 0;

	virtual boost::shared_ptr<type_specifier> build( const sasl::parser_tree::declaration_specifier& v ) = 0;

	// build statements
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::declaration_statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::if_statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::while_statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::dowhile_statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::for_statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::switch_statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::expression_statement& v) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::compound_statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::flowcontrol_statement& v ) = 0;
	virtual boost::shared_ptr<statement> build( const sasl::parser_tree::labeled_statement& v ) = 0;
};

END_NS_SASL_SYNTAX_TREE()

#endif