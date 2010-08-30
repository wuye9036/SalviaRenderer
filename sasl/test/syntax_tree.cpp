#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
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
BOOST_AUTO_TEST_CASE( type_combinator_test )
{
	using ::sasl::syntax_tree::dprog_combinator;

	using ::sasl::syntax_tree::array_type;
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::struct_type;
	using ::sasl::syntax_tree::type_specifier;
	using ::sasl::syntax_tree::variable_declaration;

	using ::sasl::syntax_tree::btc_helper;

	std::string var0_name( "var0_name" );
	std::string struct_name( "struct0_name" );

	boost::shared_ptr<struct program> prog;
	boost::shared_ptr<struct variable_declaration> vardecl1;
	dprog_combinator prog_comb("hello");

	boost::shared_ptr<type_specifier>
		var0type, var1type, var2type;
	boost::shared_ptr<struct_type> var3type;
	boost::shared_ptr<array_type> var4type;
	prog_comb
		.dvar( var0_name )
			.dtype().dbuildin( buildin_type_code::_float ).end( var0type )
		.end()
		.dvar( var0_name )
			.dtype().dvec( buildin_type_code::_uint64, 2 ).end( var1type )
		.end()
		.dvar( var0_name )
			.dtype().dmat( buildin_type_code::_double, 4, 3 ).end( var2type )
		.end()
		.dvar( var0_name )
			.dtype().dalias( struct_name ).dtypequal( type_qualifiers::_uniform ).end( var3type )
		.end()
		.dvar( var0_name )
			.dtype()
				.dbuildin( buildin_type_code::_float )
				.darray().end()
				.darray().dconstant2( (int32_t)186 ).end()
			.end( var4type )
		.end()
	.end( prog );

	BOOST_CHECK( var0type->node_class() == syntax_node_types::buildin_type );
	BOOST_CHECK( var0type && var0type->value_typecode == buildin_type_code::_float );
	BOOST_CHECK( var1type && var1type->value_typecode == btc_helper::vector_of(buildin_type_code::_uint64, 2) );
	BOOST_CHECK( var2type && var2type->value_typecode == btc_helper::matrix_of(buildin_type_code::_double, 4, 3) );
	BOOST_CHECK( var2type->node_class() == syntax_node_types::buildin_type );
	BOOST_CHECK( !var2type->is_uniform() );
	BOOST_CHECK( var3type && var3type->name->str == struct_name );
	BOOST_CHECK( var3type->is_uniform() );
	BOOST_CHECK( var3type->node_class() == syntax_node_types::struct_type );
	BOOST_CHECK( var4type );
	BOOST_CHECK( var4type->node_class() == syntax_node_types::array_type );
	BOOST_CHECK( var4type->elem_type->value_typecode == buildin_type_code::_float );
	BOOST_CHECK( var4type->array_lens.size() == 2);
	BOOST_CHECK( !var4type->array_lens[0] );
	BOOST_CHECK( var4type->array_lens[1]->node_class() == syntax_node_types::constant_expression );
}

BOOST_AUTO_TEST_CASE( expr_combinator_test ){
	using ::sasl::syntax_tree::tree_combinator;
	using ::sasl::syntax_tree::dexpr_combinator;
	using ::sasl::syntax_tree::dcast_combinator;

	using ::sasl::syntax_tree::binary_expression;
	using ::sasl::syntax_tree::call_expression;
	using ::sasl::syntax_tree::cast_expression;
	using ::sasl::syntax_tree::cond_expression;
	using ::sasl::syntax_tree::constant_expression;
	using ::sasl::syntax_tree::index_expression;
	using ::sasl::syntax_tree::member_expression;
	using ::sasl::syntax_tree::unary_expression;
	using ::sasl::syntax_tree::variable_expression;

	{
		boost::shared_ptr<constant_expression> expr_c;
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dconstant2( 0.1f )
		.end( expr_c );
		BOOST_CHECK( expr_c->node_class() == syntax_node_types::constant_expression );
		BOOST_CHECK( expr_c->ctype == literal_constant_types::real );
		BOOST_CHECK( expr_c->value_tok->str == boost::lexical_cast<std::string>(0.1f) );
	}
	
	boost::shared_ptr<constant_expression> cexpr;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dconstant2( true )
		.end( cexpr );
		BOOST_CHECK( cexpr->ctype == literal_constant_types::boolean );
		BOOST_CHECK( cexpr->value_tok->str == boost::lexical_cast<std::string>(true) );
	}

	boost::shared_ptr<variable_expression> varexpr;
	{
		std::string var_name("var0_name");
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dvarexpr( var_name )
		.end(varexpr);
		BOOST_CHECK( varexpr && varexpr->node_class() == syntax_node_types::variable_expression );
		BOOST_CHECK( varexpr->var_name->str == var_name );
	}

	boost::shared_ptr<unary_expression> unaryexpr;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dunary( operators::negative )
				.dnode( varexpr )
			.end()
		.end(unaryexpr);
		BOOST_CHECK( unaryexpr );
		BOOST_CHECK( unaryexpr->node_class() == syntax_node_types::unary_expression );
		BOOST_CHECK( unaryexpr->op == operators::negative );
		BOOST_CHECK( unaryexpr->expr == varexpr );
	}

	boost::shared_ptr<cast_expression> castexpr;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dcast()
				.dtype()
					.dbuildin( buildin_type_code::_float )
				.end()
				.dexpr()
					.dnode( varexpr )
				.end()
			.end()
		.end(castexpr);
		BOOST_CHECK( castexpr );
		BOOST_CHECK( castexpr->node_class() == syntax_node_types::cast_expression );
		BOOST_CHECK( castexpr->casted_type->value_typecode == buildin_type_code::_float );
		BOOST_CHECK( castexpr->expr == varexpr );
	}

	boost::shared_ptr<binary_expression> binexpr;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dbinary()
				.dlexpr()
					.dnode( castexpr )
				.end()
				.dop( operators::add )
				.drexpr()
					.dnode( varexpr )
				.end()
			.end()
		.end(binexpr);

		BOOST_CHECK( binexpr );
		BOOST_CHECK( binexpr->node_class() == syntax_node_types::binary_expression );
		BOOST_CHECK( binexpr->left_expr == castexpr );
		BOOST_CHECK( binexpr->right_expr == varexpr );
		BOOST_CHECK( binexpr->op == operators::add );
	}

	boost::shared_ptr<cond_expression> branchexpr;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dbranchexpr()
				.dcond().dnode( varexpr ).end()
				.dyes().dnode( unaryexpr ).end()
				.dno().dnode( binexpr ).end()
			.end()
		.end( branchexpr );

		BOOST_CHECK( branchexpr );
		BOOST_CHECK( branchexpr->node_class() == syntax_node_types::cond_expression );
		BOOST_CHECK( branchexpr->cond_expr == varexpr );
		BOOST_CHECK( branchexpr->yes_expr == unaryexpr );
		BOOST_CHECK( branchexpr->no_expr == binexpr );
	}

	boost::shared_ptr<member_expression> mem0expr;
	boost::shared_ptr<member_expression> mem1expr;
	{
		std::string m0("member0"), m1("member1");
		
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dnode(branchexpr)
			.dmember(m0).get_node(mem0expr)
			.dmember(m1)
		.end(mem1expr);

		BOOST_CHECK( mem1expr );
		BOOST_CHECK( mem1expr->node_class() == syntax_node_types::member_expression );
		BOOST_CHECK( mem1expr->member->str == m1 );
		BOOST_CHECK( mem1expr->expr->node_class() == syntax_node_types::member_expression );

		BOOST_CHECK( mem0expr->member->str == m0 );
		BOOST_CHECK( mem0expr->expr == branchexpr );
	}

	boost::shared_ptr<call_expression> callexpr;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dnode( mem0expr )
			.dcall()
				.dargument().dnode(varexpr).end()
				.dargument().dnode(castexpr).end()
			.end()
		.end(callexpr);
					
		BOOST_CHECK( callexpr );
		BOOST_CHECK( callexpr->node_class() == syntax_node_types::call_expression );
		BOOST_CHECK( callexpr->expr == mem0expr );
		BOOST_CHECK( callexpr->args[0] == varexpr );
		BOOST_CHECK( callexpr->args[1] == castexpr );
	}

	boost::shared_ptr<index_expression> indexexpr0, indexexpr1;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dnode( callexpr )
			.dindex().dnode(mem1expr).end()
			.get_node( indexexpr0 )
			.dindex().dnode(mem0expr).end()
		.end(indexexpr1);

		BOOST_CHECK( indexexpr1 );
		BOOST_CHECK( indexexpr1->node_class() == syntax_node_types::index_expression );
		BOOST_CHECK( indexexpr1->index_expr == mem0expr );
		BOOST_CHECK( indexexpr1->expr == indexexpr0 );
		BOOST_CHECK( indexexpr0 );
		BOOST_CHECK( indexexpr0->node_class() == syntax_node_types::index_expression );
		BOOST_CHECK( indexexpr0->index_expr == mem1expr );
		BOOST_CHECK( indexexpr0->expr == callexpr );
	}
}

BOOST_AUTO_TEST_SUITE_END();