#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/syntax_tree/utility.h>
#include <eflib/include/memory/lifetime_manager.h>

#define SYNCASE_(case_name) syntax_cases::instance().##case_name##()
#define SYNCASENAME_( case_name ) syntax_cases::instance().##case_name##_name()

boost::mutex semantic_cases::mtx;
boost::shared_ptr<semantic_cases> semantic_cases::tcase;

using namespace ::sasl::semantic;

void clear_semantic( SYNTAX_(node)& nd ){
	nd.semantic_info( boost::shared_ptr<SEMANTIC_(semantic_info)>() );
}

semantic_cases& semantic_cases::instance(){
	boost::mutex::scoped_lock lg(mtx);
	if ( !tcase ) {
		eflib::lifetime_manager::at_main_exit( semantic_cases::release );
		tcase.reset( new semantic_cases() );
		tcase->initialize();
	}
	return *tcase;
}

bool semantic_cases::is_avaliable()
{
	boost::mutex::scoped_lock lg(mtx);
	return tcase;
}

void semantic_cases::release(){
	boost::mutex::scoped_lock lg(mtx);
	if ( tcase ){
		if( syntax_cases::is_avaliable() ){
			follow_up_traversal( SYNCASE_(prog_for_gen), clear_semantic );
			follow_up_traversal( SYNCASE_(jit_prog), clear_semantic );
		}
		tcase.reset();
	}
}

semantic_cases::semantic_cases(){
}

void semantic_cases::initialize(){
	cim = COMMON_(compiler_info_manager)::create();
	SEMANTIC_(semantic_analysis)( SYNCASE_(prog_for_gen), cim );
	SEMANTIC_(semantic_analysis)( SYNCASE_(jit_prog), cim );

	LOCVAR_( cexpr_776uint ) = extract_semantic_info<const_value_si>( SYNCASE_(cexpr_776uint) );
	LOCVAR_( sym_f0 ) = SYNCASE_(func_flt_2p_n_gen)->symbol();
	LOCVAR_( sym_p0 ) = SYNCASE_(p0_fn0)->symbol();
	LOCVAR_( sym_p1 ) = SYNCASE_(p1_fn0)->symbol();
	LOCVAR_( sym_root ) = SYNCASE_( prog_for_gen )->symbol();
}