#ifndef SASL_PROGRAM_H
#define SASL_PROGRAM_H

#include <boost/spirit/include/classic_ast.hpp>

template <typename ScannerT>
program::definition<ScannerT>::definition( const program& self)
{
	r_program = *g_decl >> end_p;
}

template <typename ScannerT>
const RULE_TYPE(r_program)& program::definition<ScannerT>::start() const{
	return r_program;
}

#endif