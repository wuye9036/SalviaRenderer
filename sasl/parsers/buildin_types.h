#ifndef SASL_PARSER_BUILDIN_TYPES_H
#define SASL_PARSER_BUILDIN_TYPES_H

#include "parsers.h"

#include <boost/spirit/include/classic_ast.hpp>

using namespace std;
using namespace boost::spirit::classic;

#define SCALAR_TYPE_SYMBOL(type_name) ( type_name )

template<class ScannerT>
buildin_type::definition<ScannerT>::definition(const buildin_type& self){
	self.buildin_type_symbols.add
		SCALAR_TYPE_SYMBOL( "bool" )

		SCALAR_TYPE_SYMBOL( "int8" )
		SCALAR_TYPE_SYMBOL( "uint8" )
		SCALAR_TYPE_SYMBOL( "int16" )
		SCALAR_TYPE_SYMBOL( "uint16" )
		SCALAR_TYPE_SYMBOL( "int" )
		SCALAR_TYPE_SYMBOL( "int32" )
		SCALAR_TYPE_SYMBOL( "uint32" )
		SCALAR_TYPE_SYMBOL( "int64" )
		SCALAR_TYPE_SYMBOL( "uint64" )
		
		SCALAR_TYPE_SYMBOL( "half" )
		SCALAR_TYPE_SYMBOL( "float" )
		SCALAR_TYPE_SYMBOL( "double" )
		;

	r_scalar_type = leaf_node_d[ keyword_d[self.buildin_type_symbols] ];
	
	r_vector =								// sample:
		no_node_d[ str_p("vector") >> '<' ]	// vector <
		>> r_scalar_type >> no_node_d[ch_p(',')]	//  int,
		>> leaf_node_d[digit_p]				//  4
		>> no_node_d[ch_p('>')];					// >

	r_matrix =														// sample:
		no_node_d[ str_p("matrix") >> '<' ]							//  matrix <
		>> r_scalar_type											//   float
		>> no_node_d[ch_p(',')] >> digit_p >> no_node_d[ch_p(',')] >> digit_p	//   4, 4
		>> no_node_d[ch_p('>')];											//  >

	r_buildin_type = 
		r_scalar_type
		| r_vector
		| r_matrix;
}

template<class ScannerT>
const RULE_TYPE(r_buildin_type)& buildin_type::definition<ScannerT>::start() const{
	return r_buildin_type;
}

#endif