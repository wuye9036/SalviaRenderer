#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/enums_helper.h>
#include <sasl/include/syntax_tree/make_tree.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/expression.h>
#include <eflib/include/memory/lifetime_manager.h>
#include <eflib/include/platform/disable_warnings.h>
#include <boost/thread.hpp>
#include <eflib/include/platform/enable_warnings.h>

using namespace ::sasl::syntax_tree;

boost::mutex syntax_cases::mtx;
boost::shared_ptr<syntax_cases> syntax_cases::tcase;

syntax_cases& syntax_cases::instance(){
	boost::mutex::scoped_lock lg(mtx);
	if ( !tcase ) {
		eflib::lifetime_manager::at_main_exit( syntax_cases::release );
		tcase.reset( new syntax_cases() );
		tcase->initialize();
	}
	return *tcase;
}

bool syntax_cases::is_avaliable()
{
	boost::mutex::scoped_lock lg(mtx);
	return tcase;
}

void syntax_cases::release(){
	boost::mutex::scoped_lock lg(mtx);
	if ( tcase ){ tcase.reset(); }
}

syntax_cases::syntax_cases():
LOCVAR_(btc_sint8)( buildin_type_code::_sint8 ),
LOCVAR_(btc_sint32)( buildin_type_code::_sint32),
LOCVAR_(btc_uint32)( buildin_type_code::_uint32 ),
LOCVAR_(btc_uint64)( buildin_type_code::_uint64 ),
LOCVAR_(btc_double)( buildin_type_code::_double ),
LOCVAR_(btc_float)( buildin_type_code::_float ),
LOCVAR_(btc_boolean)( buildin_type_code::_boolean ),
LOCVAR_(btc_void)( buildin_type_code::_void ),
LOCVAR_(btc_short2)( sasl_ehelper::vector_of(buildin_type_code::_sint16, 2) ),
LOCVAR_(btc_float3)( sasl_ehelper::vector_of(buildin_type_code::_float, 3) ),
LOCVAR_(btc_double2x4)( sasl_ehelper::matrix_of(buildin_type_code::_double, 2, 4) ),
LOCVAR_(btc_ulong3x2)( sasl_ehelper::matrix_of(buildin_type_code::_uint64, 3, 2) ),
LOCVAR_(btc_none)( buildin_type_code::none )
{}

void syntax_cases::initialize(){
	// program
	LOCVAR_(prog_name) = std::string( "_this_is_empty_prog_test_" );
	LOCVAR_(val_3p25f) = 3.25f;
	LOCVAR_(val_17ushort) = (uint16_t)17;
	LOCVAR_(val_776uint) = 776;
	LOCVAR_(val_874uint) = 874;
	LOCVAR_(val_21uint) = 21;

	dprog_combinator( prog_name().c_str() ).end( LOCVAR_(empty_prog) );

	// create scalar types.
	dtype_combinator(NULL)
		.dbuildin(btc_sint8()) .end( LOCVAR_(type_sint8));
	dtype_combinator(NULL)
		.dbuildin(btc_sint32()) .end( LOCVAR_(type_sint32));
	dtype_combinator(NULL)
		.dbuildin(btc_uint32()).end(LOCVAR_(type_uint32));
	dtype_combinator(NULL)
		.dbuildin(btc_uint64()) .end( LOCVAR_(type_uint64));
	dtype_combinator(NULL)
		.dbuildin(btc_boolean()).end( LOCVAR_(type_boolean));
	dtype_combinator(NULL)
		.dbuildin(btc_float()) .end( LOCVAR_(type_float));
	dtype_combinator(NULL)
		.dbuildin(btc_double()) .end( LOCVAR_(type_double));
	dtype_combinator(NULL)
		.dbuildin(btc_void()) .end( LOCVAR_(type_void));

	// create vector and matrix types
	dtype_combinator(NULL)
		.dbuildin(btc_short2()).end( LOCVAR_(type_short2) );
	dtype_combinator(NULL)
		.dbuildin(btc_float3()).end( LOCVAR_(type_float3) );
	dtype_combinator(NULL)
		.dbuildin(btc_double2x4()).end(LOCVAR_(type_double2x4));
	dtype_combinator(NULL)
		.dbuildin(btc_ulong3x2()).end(LOCVAR_(type_ulong3x2));

	// create expressions
	dexpr_combinator(NULL).dconstant2( val_3p25f() ).end( LOCVAR_(cexpr_3p25f) );
	dexpr_combinator(NULL).dconstant2( val_17ushort() ).end( LOCVAR_(cexpr_17ushort) );
	dexpr_combinator(NULL).dconstant2( val_776uint() ).end( LOCVAR_(cexpr_776uint) );
	dexpr_combinator(NULL).dconstant2( val_874uint() ).end( LOCVAR_(cexpr_874uint) );
	dexpr_combinator(NULL).dconstant2( val_21uint() ).end( LOCVAR_(cexpr_21uint) );
	dexpr_combinator(NULL).dbinary()
		.dlexpr()
			.dnode( cexpr_17ushort() )
		.end()
		.dop( operators::add )
		.drexpr()
			.dnode( cexpr_3p25f() )
		.end()
	.end( LOCVAR_(expr0_add) );
	dexpr_combinator(NULL).dbinary()
		.dlexpr().dnode( cexpr_776uint() ).end()
		.dop( operators::add )
		.drexpr().dnode( cexpr_21uint() ).end()
	.end( LOCVAR_(expr1_add) );

	// create variables
	// int8_t var_int8;
	// float var_float_3p25f = 3.25f;
	dvar_combinator(NULL)
		.dname( NAME_(var_int8) )
		.dtype().dnode( type_sint8() ).end()
	.end( LOCVAR_(var_int8) );
	dvar_combinator(NULL)
		.dname( NAME_(var_float_3p25f) )
		.dtype().dnode(type_float()).end()
		.dinit_expr().dnode( cexpr_3p25f() ).end( LOCVAR_(exprinit_cexpr_3p25f) )
	.end( LOCVAR_(var_float_3p25f) );

	//////////////////////////////////////
	// create functions

	/*
		void fn0_sem_name();
	*/
	dfunction_combinator(NULL)
		.dname( NAME_(fn0_sem) )
		.dreturntype().dnode( type_void() ).end()
	.end( LOCVAR_(fn0_sem) ) ;

	/*
		uint64_t fn1_sem( uint64_t par0_0_fn1, int8_t par1_1_fn1 ){
			par0_0_fn1;
		}
	*/
	dfunction_combinator(NULL)
		.dname( NAME_(fn1_sem) )
		.dreturntype().dnode( type_float() ).end()
		.dparam()
			.dname( NAME_(par0_0_fn1) )
			.dtype().dnode( type_uint64() ).end()
		.end( LOCVAR_(par0_0_fn1) )
		.dparam()
			.dname( NAME_(par1_1_fn1) )
			.dtype().dnode( type_sint8() ).end()
		.end( LOCVAR_(par1_1_fn1) )
		.dbody()
			.dexprstmt().dvarexpr(NAME_(par0_0_fn1)).end()
		.end( LOCVAR_(fn1_body) )
	.end( LOCVAR_(fn1_sem) );
	
	dfunction_combinator(NULL)
		.dname( NAME_(fn2_sem) )
		.dreturntype().dnode(type_uint32()).end()
		.dbody()
			.dreturn_expr().dnode( expr1_add() ).end()
		.end()
	.end( LOCVAR_(fn2_sem) );
				
	dfunction_combinator(NULL)
		.dname( NAME_(fn3_jit) )
		.dreturntype().dnode( type_double() ).end()
		.dbody()
			.dreturn_expr()
				.dbinary()
					.dlexpr().dnode( expr0_add() ).end()
					.dop( operators::add )
					.drexpr().dnode( expr1_add() ).end()
				.end()
			.end()
		.end()
	.end( LOCVAR_(fn3_jit) );

	// typedef tdef0_double2x4 double2x4;
	dtypedef_combinator(NULL)
		.dname( NAME_(tdef0_double2x4) )
		.dtype().dnode( type_double2x4() ).end()
	.end( LOCVAR_(tdef0_double2x4) );

	LOCVAR_(null_prog).reset();

	dprog_combinator( NAME_(prog_for_syntax_test).c_str() )
		.dvar("").dnode( var_float_3p25f() ).end()
		.dfunction("").dnode( fn1_sem() ).end()
		.dtypedef().dnode( tdef0_double2x4() ).end()
	.end( LOCVAR_(prog_for_syntax_test) );


	dprog_combinator( NAME_(prog_for_semantic_test).c_str() )
		.dfunction("").dnode( fn0_sem() ).end()
		.dfunction("").dnode( fn1_sem() ).end()
		.dfunction("").dnode( fn2_sem() ).end()
	.end( LOCVAR_(prog_for_semantic_test) );

	dprog_combinator( NAME_(prog_for_jit_test).c_str() )
		.dfunction( "foo" )
			.dreturntype().dnode( type_sint32() ).end()
			.dbody()
				.dreturn_expr().dnode( expr1_add() ).end()
			.end()
		.end()
		.dfunction( NAME_(fn3_jit) )
			.dnode( fn3_jit() )
		.end()
	.end( LOCVAR_(prog_for_jit_test) )
	;
}