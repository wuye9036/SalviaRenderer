#ifndef SASL_PARSER_STATEMENT_H
#define SASL_PARSER_STATEMENT_H

#include "parsers.h"

template< typename ScannerT >
statement::definition<ScannerT>::definition( const statement& self){
	r_statement = 
		g_compound_statement
		| r_declaration_statement
		| r_expression_statement
		| r_if_statement
		| r_switch_statement
		| r_while_statement
		| r_do_while_statement
		| r_for_statement
		| r_null_statement
		| r_labeled_statement
		;
	
	r_null_statement = ch_p(';');
	r_declaration_statement = g_declaration;
	
	r_expression_statement = g_expression >> ch_p(';') ;
	
	r_if_statement = 
		no_node_d[ keyword_d[ str_p("if") ] ] >> 
			inner_node_d[ '(' >> g_expression >> ')' ]
		>> r_statement >> 
		!( no_node_d[ keyword_d[ str_p("else") ] ] >> r_statement );
		
		// a label has 2 components. 'case' + expression or 'default'/identifier + ':' 
	r_labeled_statement = 
		( keyword_d[ str_p("case") ] >> g_expression >> no_node_d[ ch_p(':') ]
		| keyword_d[ str_p("defalut") ] >> ch_p(':')
		| g_identifier >> ch_p(':')	) >> *r_statement;

	r_switch_statement = 
		inner_node_d[ keyword_d[ str_p("switch") ] ] 
		>> inner_node_d[ ch_p('(') >> g_expression >> ch_p(')') ]
		>> inner_node_d[ ch_p('{')	>> *r_statement >> ch_p('}') ]
		;
	
	r_while_statement = 
		no_node_d[ keyword_d[str_p("while")] ]
		>> inner_node_d[ ch_p('(') >> g_expression >> ')' ]
		>> r_statement
		;
	
	r_do_while_statement = 
		no_node_d[ keyword_d[str_p("do")] ]
		>> r_statement
		>> no_node_d[ keyword_d[str_p("while")] ]
		>> inner_node_d[ ch_p('(') >> g_expression >> ')' ]
		;
	
	r_for_statement =
		no_node_d[ keyword_d[ str_p( "for" ) ]	>> ch_p('(') ]
		>> r_for_init_statement
		>> ( r_expression_statement | r_null_statement )
		>> ! g_expression 
		>> no_node_d[ ch_p(')') ]
		>> r_statement
		;
	
	r_for_init_statement = 
		r_expression_statement
		| r_declaration_statement
		| r_null_statement
		;
	
	r_break_statement = 
		leaf_node_d[ keyword_d[ str_p("break") ] >> ch_p(';') ]
		;
		
	r_continue_statement = 
		leaf_node_d[ keyword_d[ str_p("continue") ] >> ch_p(';') ]
		;
		
	r_return_statement = 
		no_node_d[ keyword_d[ str_p("return") ] ] >> ! g_expression >>  no_node_d[ ch_p(';') ]
		;
		
	r_statement_list = *r_statement;
}

DEFINE_START_RULE( statement, r_statement )
#endif