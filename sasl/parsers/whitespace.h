#ifndef SASL_WHITESPACE_H
#define SASL_WHITESPACE_H

#include "parsers.h"

#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_ast.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>

struct forward_line{
	template< typename IteratorT >
	void operator()( const IteratorT& begin, const IteratorT& end ) const{
		//...
	}

	template< typename ValueT >
	void operator()( const ValueT& val ) const{
		//...
	}
};

template<class ScannerT>
white_space::definition<ScannerT>::definition(const white_space& self){
	r_newline_space = 		
		ch_p('\n')
		| str_p("\r\n")
		| ch_p('\r')
		| comment_p("//")
		;

	r_preprocessor_line = 
		r_newline_space[ forward_line() ]
		>> +(
			*( r_inline_white_space ) 
			>> ch_p('#') >> +( r_inline_white_space ) 
			>> str_p( "line" ) >> +( r_inline_white_space ) 
			>> +digit_p >> +( r_inline_white_space ) 
			>> ! confix_p('"', *c_escape_ch_p, '"') >> *( r_inline_white_space )
			>> r_newline_space [ forward_line() ]
		);

	r_inline_white_space = 
		space_p
		| str_p("/*") >> *( r_newline_space[ forward_line() ] | (anychar_p - str_p("*/")) ) >> str_p("*/")
		;

	r_whitespace = 
		r_preprocessor_line
		| r_inline_white_space
		| r_newline_space[ forward_line() ]
		;
	;
}


template<class ScannerT>
const RULE_TYPE(r_whitespace) & white_space::definition<ScannerT>::start() const{
	return r_whitespace;
}
#endif