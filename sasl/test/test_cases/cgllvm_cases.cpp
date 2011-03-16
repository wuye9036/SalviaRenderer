#include <sasl/test/test_cases/cgllvm_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/syntax_tree/utility.h>

#include <softart/include/enums.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/diagnostics/logrout.h>
#include <eflib/include/memory/lifetime_manager.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::shared_ptr;

#define SYNCASE_(case_name) syntax_cases::instance().case_name ()
#define SYNCASENAME_( case_name ) syntax_cases::instance(). case_name##_name()

#define SEMCASE_(case_name) semantic_cases::instance().case_name ()
#define SEMCASENAME_( case_name ) semantic_cases::instance(). case_name##_name()

boost::mutex cgllvm_cases::mtx;
boost::shared_ptr<cgllvm_cases> cgllvm_cases::tcase;

void clear_cgctxt( SYNTAX_(node)& nd, ::boost::any* ){
	nd.codegen_ctxt( boost::shared_ptr<CODEGEN_(codegen_context)>() );
}

#define CONTEXT_OF( node_name ) sasl::code_generator::extract_codegen_context<sasl::code_generator::cgllvm_common_context>( SEMCASE_(node_name)->codegen() )

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
			SYNTAX_(follow_up_traversal)( SYNCASE_( prog_for_semantic_test ), clear_cgctxt );
			SYNTAX_(follow_up_traversal)( SYNCASE_( null_prog ), clear_cgctxt );
			SYNTAX_(follow_up_traversal)( SYNCASE_( prog_for_jit_test ), clear_cgctxt );
		}
		tcase.reset();
	}
}

cgllvm_cases::cgllvm_cases(){
}

void cgllvm_cases::initialize(){
	semantic_cases::instance();

	shared_ptr< SEMANTIC_(global_si) > si_jit_root = SEMANTIC_(semantic_analysis)( SYNCASE_(prog_for_jit_test), softart::lang_vertex_sl );
	LOCVAR_(root) = CODEGEN_(generate_llvm_code)( si_jit_root );

	fputs("\n======================================================\r\n", stderr);
	fputs("Verify generated code: \r\n", stderr);

	std::vector<CODEGEN_(optimization_options)> ops;
	ops.push_back( CODEGEN_(opt_verify) );
	CODEGEN_(optimize) ( root(), ops );

	eflib::logrout::write_state( eflib::logrout::screen(), eflib::logrout::off() );

	fputs("\n======================================================\n", stderr);
	fputs("Generated LLVM IR (before optimized): \r\n", stderr);
	CODEGEN_(dump)(root());
	fputs("======================================================\n", stderr);

	ops.clear();
	ops.push_back( CODEGEN_(opt_preset_std_for_function) );
	CODEGEN_(optimize) ( root(), ops );

	fputs("\n======================================================\n", stderr);
	fputs("Generated LLVM IR (after optimized): \r\n", stderr);
	CODEGEN_(dump)(root());
	fputs("======================================================\n", stderr);
	
	eflib::logrout::write_state( eflib::logrout::screen(), eflib::logrout::on() );

	std::string err;
	LOCVAR_(jit) = CODEGEN_(cgllvm_jit_engine::create)( boost::shared_polymorphic_cast<CODEGEN_(cgllvm_global_context)>( LOCVAR_(root) ), err);
	EFLIB_ASSERT( LOCVAR_(jit), err.c_str() );
}
