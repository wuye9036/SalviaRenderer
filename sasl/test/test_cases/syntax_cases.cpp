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
		tcase->initialize();
	}
	return *tcase;
}

void syntax_cases::release(){
	boost::mutex::scoped_lock lg(mtx);
	if ( tcase ){ tcase.reset(); }
}

syntax_cases::syntax_cases():
LOCVAR_(btc_sint8)( buildin_type_code::_sint8 ),
LOCVAR_(btc_uint64)( buildin_type_code::_uint64 ),
LOCVAR_(btc_double)( buildin_type_code::_double ),
LOCVAR_(btc_float)( buildin_type_code::_float ),
LOCVAR_(btc_boolean)( buildin_type_code::_boolean ),
LOCVAR_(btc_void)( buildin_type_code::_void ),
LOCVAR_(btc_short2)( btc_helper::vector_of(buildin_type_code::_sint16, 2) ),
LOCVAR_(btc_float3)( btc_helper::vector_of(buildin_type_code::_float, 3) ),
LOCVAR_(btc_double2x4)( btc_helper::matrix_of(buildin_type_code::_double, 2, 4) ),
LOCVAR_(btc_ulong3x2)( btc_helper::matrix_of(buildin_type_code::_uint64, 3, 2) )
{}

void syntax_cases::initialize(){
	// program
	LOCVAR_(prog_name) = std::string( "_this_is_empty_prog_test_" );
	LOCVAR_(val_3p25f) = 3.25f;
	LOCVAR_(val_17ushort) = (uint16_t)17;

	dprog_combinator( prog_name().c_str() ).end( LOCVAR_(empty_prog) );

	// create scalar types.
	dtype_combinator(NULL)
		.dbuildin(btc_sint8()) .end( LOCVAR_(type_sint8));
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

	// create variables
	dvar_combinator(NULL)
		.dname( NAME_(var_int8) )
		.dtype().dnode( type_sint8() ).end()
	.end( LOCVAR_(var_int8) );
	dvar_combinator(NULL)
		.dname( NAME_(var_float_3p25f) )
		.dtype().dnode(type_float()).end()
		.dinit_expr().dnode( cexpr_3p25f() ).end( LOCVAR_(exprinit_cexpr_3p25f) )
	.end( LOCVAR_(var_float_3p25f) );

	//create functions
	dvar_combinator(NULL)
		.dname( NAME_(func_nnn) )
		.dreturntype().dnode( type_void ).end()
	.end( LOCVAR_(func_nnn) ) ;
}