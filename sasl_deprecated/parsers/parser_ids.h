#ifndef SASL_PARSER_IDS_H
#define SASL_PARSER_IDS_H

#include <boost/preprocessor.hpp>

#define PARSER_ID( r, data, i, item ) static const int item = i ;
#define PARSER_IDS( id_seq ) \
struct parser_ids{ \
	BOOST_PP_SEQ_FOR_EACH_I( PARSER_ID, 0, id_seq ) \
};

#define PARSER_SEQ \
	(r_program) \
	(r_buildin_type) \
 	(r_scalar_type)\
 	(r_vector)\
 	(r_matrix)\
\
	(r_declaration)\
 	(r_block_declaration) (r_function_definition) (r_structure_type)\
	(r_parameters) (r_parameter_item) (r_struct_definition)\
\
	(r_initialized_declarator_list)\
 	(r_initialized_declarator) \
 	(r_declarator) (r_initializer) (r_initialize_expr)\
\
 	(r_decl_spec)\
	(r_prefix_qualified_type) (r_postfix_qualified_type)\
	(r_unqualified_type) \
	(r_identifier_type)\
	(r_postfix_type_qualifier) ( r_prefix_type_qualifier )\
	(r_keyword_qualifier) (r_function_qualifier) (r_array_qualifier)\
\
	(r_expression)\
	(r_assignment_expr) (r_assignment_rhs_expr)\
 	(r_cond_expr)\
 	(r_or_expr) (r_and_expr)\
 	(r_bit_or_expr)	(r_bit_xor_expr) (r_bit_and_expr)\
 	(r_equal_expr) (r_rel_expr)\
	(r_shift_expr)\
 	(r_add_expr)\
 	(r_mul_expr)\
 	(r_cast_expr)\
 	(r_unary_expr)\
 	(r_postfix_expr)\
	(r_index_expr) (r_function_call_expr) (r_member_expr) (r_postfix_op_expr)\
 	(r_primary_expr)\
 	(r_literal_expr)\
 	(r_int_expr) (r_real_expr) (r_string_expr) (r_char_expr) (r_bool_expr) (r_type_suffix)\
 	(r_variable_expr) (r_identifier)\
 	(r_expr_list)\
\
	(r_statement)\
	(r_statement_list)\
	(r_null_statement) (r_labeled_statement)\
	(r_declaration_statement) (r_expression_statement)\
	(r_if_statement) (r_switch_statement) \
	(r_while_statement) (r_do_while_statement) (r_for_statement) ( r_for_init_statement )\
	(r_compound_statement)\
	(r_break_statement) (r_continue_statement) (r_return_statement)\
\
	(r_inline_white_space) (r_whitespace) (r_newline_space)\
	(r_preprocessor_line)
PARSER_IDS
(
	PARSER_SEQ
);

#define RULE_TYPE( rule_name ) rule<ScannerT, parser_tag< BOOST_PP_CAT(parser_ids::, rule_name) > >
#define ID_RULE( rule_name ) RULE_TYPE( rule_name ) rule_name;
#define ID_RULE_SEQ_ELEM( r, data, elem ) ID_RULE( elem )
#define ID_RULES( rule_name_seq ) BOOST_PP_SEQ_FOR_EACH( ID_RULE_SEQ_ELEM, 0, rule_name_seq )
#endif