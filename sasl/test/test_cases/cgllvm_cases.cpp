#include <sasl/test/test_cases/cgllvm_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>

#define SYNCASE_(case_name) syntax_cases::instance().##case_name##()
#define SYNCASENAME_( case_name ) syntax_cases::instance().##case_name##_name()

boost::mutex cgllvm_cases::mtx;
boost::shared_ptr<cgllvm_cases> cgllvm_cases::tcase;

#define CONTEXT_OF( node_name ) sasl::code_generator::extract_codegen_context<sasl::code_generator::cgllvm_common_context>( SYNCASE_(node_name) )

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
	LOCVAR_(null_root) = CODEGEN_(generate_llvm_code)( SYNCASE_( null_prog ) );

	LOCVAR_(type_void) = CONTEXT_OF( type_void );
}