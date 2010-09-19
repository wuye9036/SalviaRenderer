#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/include/semantic/semantic_analyser.h>

#define SYNCASE_(case_name) syntax_cases::instance().##case_name##()
#define SYNCASENAME_( case_name ) syntax_cases::instance().##case_name##_name()

boost::mutex semantic_cases::mtx;
boost::shared_ptr<semantic_cases> semantic_cases::tcase;

semantic_cases& semantic_cases::instance(){
	boost::mutex::scoped_lock lg(mtx);
	if ( !tcase ) {
		tcase.reset( new semantic_cases() );
		tcase->initialize();
	}
	return *tcase;
}

void semantic_cases::release(){
	boost::mutex::scoped_lock lg(mtx);
	if ( tcase ){ tcase.reset(); }
}

semantic_cases::semantic_cases(){
}

void semantic_cases::initialize(){
	cim = COMMON_(compiler_info_manager)::create();
	SEMANTIC_(semantic_analysis)( SYNCASE_(prog_for_gen), cim );
}