#include "parsers.h"
#include <boost/spirit/include/classic_ast.hpp>

template< typename ScannerT >
initialized_declarator_list::definition<ScannerT>::definition(const initialized_declarator_list& self)
{
	r_initialized_declarator_list = g_init_declarator >> * (',' >> g_init_declarator );
}

template< typename ScannerT >
const RULE_TYPE(r_initialized_declarator_list)& initialized_declarator_list::definition<ScannerT>::start() const{
	return r_initialized_declarator_list;
}