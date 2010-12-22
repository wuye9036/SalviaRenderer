#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/utility.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/memory/lifetime_manager.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

using namespace ::sasl::semantic;
using boost::shared_ptr;
using std::vector;

#define SYNCASE_(case_name) syntax_cases::instance().case_name ()
#define SYNCASENAME_(case_name) syntax_cases::instance().case_name##_name()

#define CHECK_RET( expr ) if( !(expr) ) return;
boost::mutex semantic_cases::mtx;
boost::shared_ptr<semantic_cases> semantic_cases::tcase;

void clear_semantic( SYNTAX_(node)& nd, ::boost::any* ){
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
			follow_up_traversal( SYNCASE_(prog_for_semantic_test), clear_semantic );
			follow_up_traversal( SYNCASE_(prog_for_jit_test), clear_semantic );
		}
		tcase.reset();
	}
}

semantic_cases::semantic_cases() : LOCVAR_(op_bexpr0)( operators::none )
{
}

void semantic_cases::initialize(){
	CHECK_RET( LOCVAR_(si_root) = SEMANTIC_(semantic_analysis)( SYNCASE_(prog_for_semantic_test) ) );
	CHECK_RET( LOCVAR_(sym_root) = LOCVAR_(si_root)->root() );
	
	vector< shared_ptr<symbol> > sym_fn0_sem_ol = LOCVAR_(sym_root)->find_overloads( SYNCASENAME_(fn0_sem) );
	CHECK_RET( sym_fn0_sem_ol.size() == 1 );
	CHECK_RET( LOCVAR_(sym_fn0_sem) = sym_fn0_sem_ol[0] );
	LOCVAR_(mangled_fn0_name) = std::string("M") + SYNCASENAME_(fn0_sem) + std::string("@@");
	LOCVAR_(mangled_fn1_name) = std::string("M") + SYNCASENAME_(fn1_sem) + std::string("@@QBU8@@") + std::string("QBS1@@");

	vector< shared_ptr<symbol> > sym_fn1_sem_ol = LOCVAR_(sym_root)->find_overloads( SYNCASENAME_(fn1_sem) );
	CHECK_RET( sym_fn1_sem_ol.size() == 1 );
	CHECK_RET( LOCVAR_(sym_fn1_sem) = sym_fn1_sem_ol[0] );

	vector< shared_ptr<symbol> > sym_fn2_sem_ol = LOCVAR_(sym_root)->find_overloads( SYNCASENAME_(fn2_sem) );
	CHECK_RET( sym_fn2_sem_ol.size() == 1 );
	CHECK_RET( LOCVAR_(sym_fn2_sem) = sym_fn2_sem_ol[0] );
	CHECK_RET( LOCVAR_(fn2_sem) = sym_fn2_sem()->node()->typed_handle<function_type>() );

	CHECK_RET( LOCVAR_(fn0_sem) = sym_fn0_sem()->node()->typed_handle< SYNTAX_(function_type) >() );
	CHECK_RET( LOCVAR_(fn1_sem) = sym_fn1_sem()->node()->typed_handle< SYNTAX_(function_type) >() );

	CHECK_RET( LOCVAR_(sym_par0_0_fn1) = LOCVAR_(sym_fn1_sem)->find( SYNCASENAME_(par0_0_fn1) ) );
	CHECK_RET( LOCVAR_(sym_par1_1_fn1) = LOCVAR_(sym_fn1_sem)->find( SYNCASENAME_(par1_1_fn1) ) );
	CHECK_RET( LOCVAR_(par0_0_fn1) = LOCVAR_(sym_par0_0_fn1)->node()->typed_handle< SYNTAX_(parameter) >() );
	CHECK_RET( LOCVAR_(par1_1_fn1) = LOCVAR_(sym_par1_1_fn1)->node()->typed_handle< SYNTAX_(parameter) >() );

	CHECK_RET( LOCVAR_(body_fn2) = fn2_sem()->body );
	CHECK_RET( LOCVAR_(sym_body_fn2) = body_fn2()->symbol() );
	CHECK_RET( LOCVAR_(jstmt0_0_fn2) = body_fn2()->stmts[0]->typed_handle< SYNTAX_(jump_statement) >() );
	CHECK_RET( LOCVAR_(bexpr0_0_jstmts0) = jstmt0_0_fn2()->jump_expr->typed_handle< SYNTAX_(binary_expression) >() );
	CHECK_RET( LOCVAR_(cexpr0_l_bexpr0) = bexpr0_0_jstmts0()->left_expr->typed_handle< SYNTAX_(constant_expression) >() );
	CHECK_RET( LOCVAR_(si_cexpr0) = SEMANTIC_(extract_semantic_info)< SEMANTIC_(const_value_si) >( cexpr0_l_bexpr0() ) );
	LOCVAR_(op_bexpr0) = bexpr0_0_jstmts0()->op;
}
