#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/enums/buildin_type_code.h>
#include <sasl/include/syntax_tree/make_tree.h>
#include <boost/thread.hpp>

using namespace ::sasl::syntax_tree;

boost::mutex syntax_cases::mtx;
boost::shared_ptr<syntax_cases> syntax_cases::tcase;

syntax_cases& syntax_cases::instance(){
	boost::mutex::scoped_lock lg(mtx);
	if ( !tcase ) {
		tcase.reset( new syntax_cases() );
	}
	tcase->initialize();
	return *tcase;
}

void syntax_cases::release(){
	boost::mutex::scoped_lock lg(mtx);
	if ( tcase ){ tcase.reset(); }
}

syntax_cases::syntax_cases(){}

void syntax_cases::initialize(){
	LOCVAR_(prog_name) = std::string( "_this_is_empty_prog_test_" );
	dprog_combinator(NULL).dname( LOCVAR_(prog_name) ).end( LOCVAR_(empty_prog) );

	dtype_combinator(NULL)
		.dbuildin( buildin_type_code::_sint8) .end( LOCVAR_(type_sint8));
	dtype_combinator(NULL)
		.dbuildin(buildin_type_code::_uint64) .end( LOCVAR_(type_uint64));
	dtype_combinator(NULL)
		.dbuildin(buildin_type_code::_boolean).end( LOCVAR_(type_boolean));
	dtype_combinator(NULL)
		.dbuildin(buildin_type_code::_double) .end( LOCVAR_(type_double));
	dtype_combinator(NULL)
		.dbuildin(buildin_type_code::_void  ) .end( LOCVAR_(type_void));
}