#ifndef SASL_PARSER_PARAMETER_DECLARATION_PART
#define SASL_PARSER_PARAMETER_DECLARATION_PART

#include "parsers.h"

template< typename ScannerT>
parameter_declaration_part::definition<ScannerT>::definition(const parameter_declaration_part &self){
	r_parameter_declaration_part = '(' >> ! r_parameter_declaration_list >> ')';
	r_parameter_declaration_list = 
		r_parameter_declaration >> *( ch_p(',') >> r_parameter_declaration );
	r_parameter_declaration = 
		g_declaration_specifier >> !( r_identifier >> !( ch_p('=') >> g_assign_expr ) );
}



#endif