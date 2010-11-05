#include <sasl/test/test_cases/cgllvm_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/syntax_tree/utility.h>
#include <eflib/include/memory/lifetime_manager.h>

#include <boost/any.hpp>

#define SYNCASE_(case_name) syntax_cases::instance().##case_name##()
#define SYNCASENAME_( case_name ) syntax_cases::instance().##case_name##_name()

boost::mutex cgllvm_cases::mtx;
boost::shared_ptr<cgllvm_cases> cgllvm_cases::tcase;

void clear_cgctxt( SYNTAX_(node)& nd, ::boost::any* ){
	nd.codegen_ctxt( boost::shared_ptr<CODEGEN_(codegen_context)>() );
}

#define CONTEXT_OF( node_name ) sasl::code_generator::extract_codegen_context<sasl::code_generator::cgllvm_common_context>( SYNCASE_(node_name) )

cgllvm_cases& cgllvm_cases::instance(){
	boost::mutex::scoped_lock lg(mtx);
	if ( !tcase ) {
		eflib::lifetime_manager::at_main_exit( cgllvm_cases::release );
		tcase.reset( new cgllvm_cases() );
		tcase->initialize();
	}
	return *tcase;
}

bool cgllvm_cases::is_avaliable()
{
	boost::mutex::scoped_lock lg(mtx);
	return tcase;
}

void cgllvm_cases::release(){
	boost::mutex::scoped_lock lg(mtx);
	if ( tcase ){ 
		tcase->LOCVAR_(jit).reset();
		if( syntax_cases::is_avaliable() ){
			SYNTAX_(follow_up_traversal)( SYNCASE_( prog_for_gen ), clear_cgctxt );
			SYNTAX_(follow_up_traversal)( SYNCASE_( null_prog ), clear_cgctxt );
			SYNTAX_(follow_up_traversal)( SYNCASE_( jit_prog ), clear_cgctxt );
		}
		tcase.reset();
	}
}

cgllvm_cases::cgllvm_cases(){
}

void cgllvm_cases::initialize(){
	semantic_cases::instance();

	LOCVAR_(root) = CODEGEN_(generate_llvm_code)( SYNCASE_( prog_for_gen ) );
	LOCVAR_(null_root) = CODEGEN_(generate_llvm_code)( SYNCASE_( null_prog ) );
	LOCVAR_(jit_prog) = CODEGEN_(generate_llvm_code)( SYNCASE_(jit_prog) );

	LOCVAR_(type_void) = CONTEXT_OF( type_void );
	LOCVAR_(type_float) = CONTEXT_OF( type_float );

	LOCVAR_(func_flt_2p_n_gen) = CONTEXT_OF( func_flt_2p_n_gen );
	LOCVAR_(func_flt_2p_n_gen_p0) = CONTEXT_OF( p0_fn0 );
	LOCVAR_(func_flt_2p_n_gen_p1) = CONTEXT_OF( p1_fn0 );

	std::string err;
	LOCVAR_(jit) = CODEGEN_(cgllvm_jit_engine::create)( boost::shared_polymorphic_cast<CODEGEN_(cgllvm_global_context)>( LOCVAR_(jit_prog) ), err);
}