#ifndef SASL_PARSER_ASSIGNMENT_EXPRESSION_H
#define SASL_PARSER_ASSIGNMENT_EXPRESSION_H

#include "parsers.h"

#include <boost/spirit/include/classic_if.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include <boost/spirit/include/classic_ast.hpp>

template< typename ScannerT >
assignment_expression::definition<ScannerT>::definition(const assignment_expression &self)
{
	//initialize symbols
	assign_ops.add
		("+=") ("-=") ("*=") ("/=")
		("&=") ("|=") ("^=")
		("<<=") (">>=") ("=")
		;

	rel_ops.add
		(">") (">=") ("<=") ("<");

	shift_ops.add
		("<<") (">>");

	add_ops.add
		("+") ("-");

	mul_ops.add
		("*") ("/");

	////////////////
	//set rules.
	////////////////
	r_assignment_expr = 
		r_assignment_rhs_expr >> *(assign_ops >> r_assignment_rhs_expr);

	r_assignment_rhs_expr = 
		r_cond_expr
		| r_or_expr;

	r_cond_expr =
		r_or_expr >> '?' >> g_expression >> ':' >> r_assignment_expr;
	
	r_or_expr = 
		r_and_expr >> * ( "||" >> r_and_expr );

	r_and_expr = 
		r_bit_or_expr >> *( "&&" >> r_bit_or_expr );

	r_bit_or_expr =
		r_bit_xor_expr >> *( "|" >> r_bit_xor_expr );

	r_bit_xor_expr =
		r_bit_and_expr >> * ( "^" >> r_bit_and_expr );

	r_bit_and_expr = 
		r_equal_expr >> * ( "&" >> r_equal_expr );

	r_equal_expr = 
		r_rel_expr >> * ( "==" >> r_rel_expr );

	r_rel_expr = 
		r_shift_expr >> * ( rel_ops >> r_shift_expr );

	r_shift_expr = 
		r_add_expr >> * ( shift_ops >> r_add_expr );

	r_add_expr = 
		r_mul_expr >> * ( add_ops >> r_mul_expr );

	r_mul_expr = 
		r_cast_expr >> * ( mul_ops >> r_cast_expr );

	r_cast_expr = 
		r_unary_expr
		| no_node_d[ch_p('(')] >> g_decl_spec >> no_node_d[ch_p(')')] >> g_expression
		;

	r_unary_expr = 
		r_postfix_expr
		| str_p("++") >> r_cast_expr
		| str_p("--") >> r_cast_expr
		| ch_p('+') >> r_cast_expr
		| ch_p('-') >> r_cast_expr
		| ch_p('!') >> r_cast_expr
		| ch_p('~') >> r_cast_expr
		;

	r_postfix_expr = 
		r_primary_expr >> 
		*(
			r_index_expr
			| r_function_call_expr
			| r_member_expr
			| r_postfix_op_expr
		);

	r_index_expr = inner_node_d[ ch_p('[') >> g_expression >> ']' ];
	r_function_call_expr = 
		inner_node_d[ ch_p('(') >> g_expression_list >> ')' ]
		| no_node_d[ ch_p('(') >> ')' ];
	r_member_expr = discard_first_node_d[ ch_p('.') >> g_identifier ];
	r_postfix_op_expr = leaf_node_d[ str_p("++") | str_p("--") ];

	r_primary_expr = 
		r_literal_expr
		| r_variable_expr
		| inner_node_d[ ch_p('(') >> g_expression >> (')')]
		;

	r_variable_expr = 
		g_identifier;

	r_literal_expr =
		lexeme_d[
			longest_d [
				r_int_expr			/*decimal*/
				| r_real_expr		/* real */
			]
			| r_string_expr			/*string*/
			| r_char_expr			/* char */
			| r_bool_expr			/* boolean */
		];
	
	r_string_expr = 
		leaf_node_d[ lexeme_d[ confix_p('"', *c_escape_ch_p, '"') ] ];

	r_char_expr =
		leaf_node_d[ lexeme_d[ confix_p('\'', c_escape_ch_p, '\'') ] ];

	r_int_expr = 
		leaf_node_d[ 
			( if_p(as_lower_d["0x"])[
				r_hex_expr
			].else_p[
				if_p( as_lower_d['o'] )[
					r_oct_expr
				].else_p[
					r_decimal_expr
				]
			] )] 
		>> ! leaf_node_d[ as_lower_d[ ( str_p("u") | "l" | "ul" | "lu" ) ] ];

	r_hex_expr = lexeme_d[+hex_p];
	r_oct_expr = lexeme_d[+oct_p];
	r_decimal_expr = lexeme_d[+digit_p];

	r_frac_part =
		lexeme_d[
			*digit_p >> '.' >> +digit_p
			| +digit_p >> '.'
		]
		;

	r_exponent_part = 
		lexeme_d[
			( ch_p('e') | 'E' ) >> ! sign_p >> +digit_p
				];

	r_real_expr = 
			leaf_node_d[
				( r_frac_part >> !r_exponent_part
				|  +digit_p >> r_exponent_part )
			]
			>> ! leaf_node_d[as_lower_d[ ( str_p("f") | "l" ) ]]
		
        ;

}

template< typename ScannerT >
const RULE_TYPE(r_assignment_expr)& assignment_expression::definition<ScannerT>::start() const{
	return r_assignment_expr;
}


#endif