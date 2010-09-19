#include <sasl/test/test_cases/cgllvm_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>

#define SYNCASE_(case_name) syntax_cases::instance().##case_name##()
#define SYNCASENAME_( case_name ) syntax_cases::instance().##case_name##_name()

boost::mutex cgllvm_cases::mtx;
boost::shared_ptr<cgllvm_cases> cgllvm_cases::tcase;

cgllvm_cases& cgllvm_cases::instance(){
	boost::mutex::scoped_lock lg(mtx);
	if ( !tcase ) {
		tcase.reset( new cgllvm_cases() );
		tcase->initialize();
	}
	return *tcase;
}

void cgllvm_cases::release(){
	boost::mutex::scoped_lock lg(mtx);
	if ( tcase ){ tcase.reset(); }
}

cgllvm_cases::cgllvm_cases(){
}

void cgllvm_cases::initialize(){
	semantic_cases::instance();

	LOCVAR_(root) = CODEGEN_(generate_llvm_code)( SYNCASE_( prog_for_gen ) );
}