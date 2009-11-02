#ifndef SASL_PARSER_H_INCLUDED
#define SASL_PARSER_H_INCLUDED

#include "utilities.h"
#include "parser_ids.h"
#include "../utility/declare_handle.h"

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/spirit/include/classic_distinct.hpp>

#include <vector>

using namespace boost::spirit::classic;

DECL_STRUCT_HANDLE( buildin_type );

DECL_STRUCT_HANDLE( declaration_specifier );
DECL_STRUCT_HANDLE( initialized_declarator );
DECL_STRUCT_HANDLE( initialized_declarator_list );

DECL_STRUCT_HANDLE( declaration );
DECL_STRUCT_HANDLE( structure_type );
DECL_STRUCT_HANDLE( expression );
DECL_STRUCT_HANDLE( assignment_expression );
DECL_STRUCT_HANDLE( expression_list );

struct compound_statement;
struct statement;

const distinct_parser<> keyword_p("0-9a-zA-Z_");
const distinct_directive<> keyword_d("0-9a-zA-Z_");

#include <iostream>
struct GlobalTypes{
	typedef const char* InputIterator;
};


GRAMMAR_BEGIN( identifier )
	ID_RULES(
		(r_identifier)
	);
GRAMMAR_END( r_identifier )

struct structure_type: grammar<structure_type> {
	structure_type( );
	template< typename ScannerT >
	struct definition{
		definition( const structure_type& self);

		ID_RULES( 
			(r_structure_type)
		);

		identifier g_identifier;
		declaration g_declaration;

		const RULE_TYPE(r_structure_type)& start() const;
	};
};

struct declaration: grammar<declaration>{
	declaration();
	template<class ScannerT>
	struct definition{
		definition( const declaration& self);

		ID_RULES(
			(r_declaration)
			(r_block_declaration)
			(r_function_definition) (r_struct_definition)
			(r_parameters) (r_parameter_item)
		);

		declaration_specifier g_decl_spec;
		initialized_declarator_list g_init_decl_list;
		compound_statement g_compound_statement;
		structure_type g_struct_type;
		identifier g_identifier;
		assignment_expression g_assign_expr;

		const RULE_TYPE(r_declaration)& start() const;
	};
};

struct initialized_declarator: grammar<initialized_declarator>{
	initialized_declarator();
	template<typename ScannerT>
	struct definition{
		definition( const initialized_declarator& self);

		ID_RULES(
			(r_initialized_declarator)
			(r_declarator) (r_initializer) (r_initialize_expr)
			);

		const RULE_TYPE(r_initialized_declarator)& start() const;

		assignment_expression g_assign_expr;
		expression_list g_expr_list;
		identifier g_identifier;
	};
};

struct initialized_declarator_list: grammar<initialized_declarator_list>{
	initialized_declarator_list( );
	template<typename ScannerT>
	struct definition{
		definition( const initialized_declarator_list& self);

		ID_RULES(
			(r_initialized_declarator_list)
			);

		const RULE_TYPE(r_initialized_declarator_list)& start() const;

		initialized_declarator g_init_declarator;
	};
};

struct declaration_specifier: grammar<declaration_specifier>{
	declaration_specifier();

	template<class ScannerT>
	struct definition{
		definition(const declaration_specifier& self);

		ID_RULES(
			(r_decl_spec)
			(r_prefix_qualified_type) (r_postfix_qualified_type)
			(r_unqualified_type) 
			(r_identifier_type)
			(r_postfix_type_qualifier) ( r_prefix_type_qualifier )
			(r_keyword_qualifier) (r_function_qualifier) (r_array_qualifier)
			);
		
		buildin_type g_buildin_type;
		structure_type g_struct_type;
		expression g_expression;
		identifier g_identifier;
		expression_list g_expr_list;

		const RULE_TYPE(r_decl_spec)& start() const;
	};

	symbols<> type_qual_symbols;
};

struct buildin_type: grammar<buildin_type>{
	buildin_type();
	template<class ScannerT>
	struct definition{
		definition(const buildin_type& self);
		
		ID_RULES(
			(r_buildin_type)
			(r_scalar_type) (r_vector) (r_matrix)
		);

		const RULE_TYPE(r_buildin_type) & start() const;
	};

	symbols<> buildin_type_symbols;
};

struct semantic: grammar<semantic>{
	template<class ScannerT>
	struct definition{
		definition(const semantic& self);
		rule<ScannerT> r_semantic;
		const rule<ScannerT> & start() const;
	};
};

class unit;
struct white_space: grammar<white_space>{
	white_space(){};
	white_space(unit* punit);

	template<class ScannerT>
	struct definition{
		definition(const white_space& self);
		
		ID_RULES( 
			(r_inline_white_space) (r_whitespace)
			(r_newline_space)
			(r_preprocessor_line)
		);

		const RULE_TYPE(r_whitespace) & start() const;
	};

	unit* punit_;
};

struct assignment_expression: grammar< assignment_expression >{
	assignment_expression();

	template< typename ScannerT >
	struct definition{
		definition( const assignment_expression& self );

		ID_RULES(
			(r_assignment_expr) (r_assignment_rhs_expr)
			(r_cond_expr)
			(r_or_expr) (r_and_expr)
			(r_bit_or_expr) (r_bit_xor_expr) (r_bit_and_expr)
			(r_equal_expr) (r_rel_expr)
			(r_shift_expr)
			(r_add_expr) (r_mul_expr)
			(r_cast_expr) (r_unary_expr)
			(r_postfix_expr) 
			(r_index_expr) (r_function_call_expr) (r_member_expr) (r_postfix_op_expr)
			(r_primary_expr)
			(r_variable_expr) 
			(r_literal_expr)
			(r_string_expr) (r_char_expr) (r_int_expr) (r_real_expr) (r_bool_expr)
			);

		rule<ScannerT> r_decimal_expr, r_hex_expr, r_oct_expr, r_frac_part, r_exponent_part;

		expression g_expression;
		expression_list g_expression_list;
		declaration_specifier g_decl_spec;
		identifier g_identifier;

		symbols<> 
			assign_ops,
			rel_ops,
			shift_ops,
			add_ops,
			mul_ops;

		symbols<>
			int_suffixes;

		symbols<>
			real_suffixes;

		const RULE_TYPE(r_assignment_expr)& start() const;
	};
};

struct expression_list: grammar< expression_list >{
	expression_list();
	
	template< typename ScannerT >
	struct definition{
		definition( const expression_list& self );

		ID_RULES(
			(r_expr_list)
		);

		assignment_expression g_assign_expr;
		
		const RULE_TYPE(r_expr_list)& start() const;
	};
};

struct expression: grammar< expression >
{
	expression();
	
	template< typename ScannerT >
	struct definition{
		definition( const expression& self );

		ID_RULES(
			(r_expression)
		);

		expression_list g_expr_list;

		const RULE_TYPE(r_expression)& start() const;
	};
};


GRAMMAR_BEGIN( compound_statement )
	ID_RULES( (r_compound_statement) );
	statement g_stmt;
GRAMMAR_END( r_compound_statement )

struct statement: grammar<statement>{
	statement();

	template< typename ScannerT >
	struct definition{
		definition( const statement& self );
		
		ID_RULES(
			(r_statement)
			(r_statement_list)
			(r_null_statement) (r_labeled_statement)
			(r_declaration_statement) (r_expression_statement)
			(r_if_statement) (r_switch_statement) 
			(r_while_statement) (r_do_while_statement) (r_for_statement) ( r_for_init_statement )
			(r_break_statement) (r_continue_statement) (r_return_statement)
			);

		const RULE_TYPE(r_statement)& start() const;
		
		expression g_expression;
		declaration g_declaration;
		compound_statement g_compound_statement;
		identifier g_identifier;
	};
};

struct program: grammar<program>{
	program();

	template<class ScannerT> struct definition{
		definition( const program& self);

		ID_RULE( r_program );
		const RULE_TYPE(r_program)& start() const;

		declaration g_decl;
	};
};
#endif