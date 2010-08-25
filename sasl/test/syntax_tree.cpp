#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/make_tree.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( program_test );

BOOST_AUTO_TEST_CASE( create_program ){
	using ::sasl::syntax_tree::create_node;
	using ::sasl::syntax_tree::program;

	create_node<program>( "test" );
}

BOOST_AUTO_TEST_CASE( prog_combinator_test )
{
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::dprog_combinator;

	boost::shared_ptr<struct program> prog;
	
	dprog_combinator prog_comb("hello");

	prog_comb.end( prog );

	BOOST_CHECK( prog.get() != NULL );
	BOOST_CHECK( prog->name == std::string( "hello" ) );
}

BOOST_AUTO_TEST_CASE( btc_test )
{
	using ::sasl::syntax_tree::btc_helper;

	buildin_type_code btc_float( buildin_type_code::_float );
	BOOST_CHECK( btc_float == buildin_type_code::_float );
	BOOST_CHECK( btc_helper::is_scalar( btc_float ) );
	BOOST_CHECK( !btc_helper::is_vector(btc_float) );
	BOOST_CHECK( !btc_helper::is_matrix(btc_float) );

	buildin_type_code btc_float3( btc_helper::vector_of( buildin_type_code::_float, 3) );
	BOOST_CHECK( btc_float3 != buildin_type_code::_float );
	BOOST_CHECK( !btc_helper::is_scalar( btc_float3 ) );
	BOOST_CHECK( btc_helper::is_vector(btc_float3) );
	BOOST_CHECK( !btc_helper::is_matrix(btc_float3) );
	BOOST_CHECK( btc_helper::scalar_of(btc_float3) == buildin_type_code::_float );
	BOOST_CHECK( btc_helper::dim0_len(btc_float3) == 3 );

	buildin_type_code btc_sint34( btc_helper::matrix_of( buildin_type_code::_sint32, 3, 4) );
	BOOST_CHECK( !btc_helper::is_scalar( btc_sint34 ) );
	BOOST_CHECK( !btc_helper::is_vector(btc_sint34) );
	BOOST_CHECK( btc_helper::is_matrix(btc_sint34) );
	BOOST_CHECK( btc_helper::scalar_of(btc_sint34) == buildin_type_code::_sint32 );
	BOOST_CHECK( btc_helper::dim0_len(btc_sint34) == 3 );
	BOOST_CHECK( btc_helper::dim1_len(btc_sint34) == 4 );
}

BOOST_AUTO_TEST_CASE( var_combinator_test )
{
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::dprog_combinator;
	using ::sasl::syntax_tree::dvar_combinator;

	using ::sasl::syntax_tree::type_specifier;
	using ::sasl::syntax_tree::variable_declaration;

	std::string var0_name( "var0_name" );

	boost::shared_ptr<struct program> prog;
	dprog_combinator prog_comb("hello");

	boost::shared_ptr<variable_declaration> var0decl;
	boost::shared_ptr<type_specifier> var0type;
	prog_comb
		.dvar( var0_name )
			.dtype().dbuildin( buildin_type_code::_float ).end( var0type )
		.end( var0decl )
	.end( prog );

	BOOST_CHECK( prog );
	BOOST_CHECK( var0decl );
	BOOST_CHECK( var0decl->name->str == var0_name );
	BOOST_CHECK( var0decl->type_info->value_typecode == buildin_type_code::_float );
	BOOST_CHECK( var0type->value_typecode == buildin_type_code::_float );
}

BOOST_AUTO_TEST_SUITE_END();