#include <sasl/enums/enums_utility.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/make_tree.h>

#include <sasl/test/test_cases/syntax_cases.h>

#include <boost/test/unit_test.hpp>

using namespace sasl::utility;

#define SYNCASE_(case_name) syntax_cases::instance().case_name()
#define SYNCASENAME_( case_name ) syntax_cases::instance().case_name##_name()

BOOST_AUTO_TEST_SUITE( program_test );

BOOST_AUTO_TEST_CASE( prog_combinator_test )
{
	BOOST_CHECK( SYNCASE_(empty_prog) );
	BOOST_CHECK( SYNCASE_(empty_prog)->name == SYNCASE_(prog_name) );
}

BOOST_AUTO_TEST_CASE( btc_test )
{
	builtin_types btc_double( syntax_cases::instance().btc_double() );
	BOOST_CHECK( btc_double == builtin_types::_double );
	BOOST_CHECK( is_scalar( btc_double ) );
	BOOST_CHECK( !is_vector(btc_double) );
	BOOST_CHECK( !is_matrix(btc_double) );
	BOOST_CHECK( is_real(btc_double) );
	BOOST_CHECK( !is_real( SYNCASE_(btc_sint8) ));
	BOOST_CHECK( vector_size( SYNCASE_(btc_sint8) ) == 1 );
	BOOST_CHECK( vector_count( SYNCASE_(btc_sint8) ) == 1 );
	BOOST_CHECK( is_integer( SYNCASE_(btc_sint8) ) );
	BOOST_CHECK( !is_integer( SYNCASE_(btc_double) ) );
	BOOST_CHECK( !is_integer( SYNCASE_(btc_void) ) );

	BOOST_CHECK( is_signed( SYNCASE_(btc_sint8) ) );
	BOOST_CHECK( !is_signed( SYNCASE_(btc_uint64) ) );
	BOOST_CHECK( is_unsigned( SYNCASE_(btc_uint64) ) );
	BOOST_CHECK( !is_unsigned( SYNCASE_(btc_sint8) ) );
	BOOST_CHECK( !is_signed( SYNCASE_(btc_double) ) );
	BOOST_CHECK( !is_unsigned( SYNCASE_(btc_double) ) );
	BOOST_CHECK( !is_signed( SYNCASE_(btc_void) ) );
	BOOST_CHECK( !is_unsigned( SYNCASE_(btc_void) ) );

	builtin_types btc_float3( syntax_cases::instance().btc_float3() );
	BOOST_CHECK( btc_float3 != builtin_types::_float );
	BOOST_CHECK( !is_scalar( btc_float3 ) );
	BOOST_CHECK( is_vector(btc_float3) );
	BOOST_CHECK( !is_matrix(btc_float3) );
	BOOST_CHECK( scalar_of(btc_float3) == builtin_types::_float );
	BOOST_CHECK( vector_size(btc_float3) == 3 );

	builtin_types btc_ulong3x2( syntax_cases::instance().btc_ulong3x2() );
	BOOST_CHECK( !is_scalar( btc_ulong3x2 ) );
	BOOST_CHECK( !is_vector(btc_ulong3x2) );
	BOOST_CHECK( is_matrix(btc_ulong3x2) );
	BOOST_CHECK( scalar_of(btc_ulong3x2) == builtin_types::_uint64 );
	BOOST_CHECK( vector_size(btc_ulong3x2) == 3 );
	BOOST_CHECK( vector_count(btc_ulong3x2) == 2 );

	BOOST_CHECK( !is_scalar( SYNCASE_(btc_void) ) );
	BOOST_CHECK( !is_scalar( SYNCASE_(btc_none) ) );

	BOOST_CHECK( storage_size(SYNCASE_(btc_void)) == 0 );
	BOOST_CHECK( storage_size(SYNCASE_(btc_none)) == 0 );
	BOOST_CHECK( storage_size(SYNCASE_(btc_sint8)) == 1 );
	BOOST_CHECK( storage_size(SYNCASE_(btc_short2)) == 4 );
	BOOST_CHECK( storage_size(SYNCASE_(btc_float3)) == 12 );
	BOOST_CHECK( storage_size(SYNCASE_(btc_ulong3x2)) == 48 );
	BOOST_CHECK( storage_size(SYNCASE_(btc_double2x4)) == 64 );

	BOOST_CHECK( is_standard( SYNCASE_(btc_float) ) );
	BOOST_CHECK( is_standard( SYNCASE_(btc_double) ) );
	BOOST_CHECK( is_standard( SYNCASE_(btc_double2x4) ) );
	BOOST_CHECK( is_standard( SYNCASE_(btc_float3) ) );
	BOOST_CHECK( !is_standard( SYNCASE_(btc_sint8) ) );
	BOOST_CHECK( !is_standard( SYNCASE_(btc_void) ) );
	BOOST_CHECK( is_standard( SYNCASE_(btc_float) ) );
}

BOOST_AUTO_TEST_CASE( decl_combinator_test )
{
	using sasl::syntax_tree::expression_initializer;
	using sasl::syntax_tree::member_initializer;

	BOOST_CHECK( SYNCASENAME_(var_float_3p25f) == std::string("var_float_3p25f") );

	BOOST_CHECK( SYNCASE_(prog_for_syntax_test) );
	BOOST_CHECK( SYNCASE_(prog_for_syntax_test)->decls.size() == 3 );

	BOOST_CHECK( SYNCASE_(prog_for_syntax_test)->decls[0] == SYNCASE_(var_float_3p25f) );
	BOOST_REQUIRE( SYNCASE_(var_float_3p25f) );
	BOOST_REQUIRE( SYNCASE_(var_float_3p25f)->declarators.size() == 1 );
	BOOST_CHECK( SYNCASE_(var_float_3p25f)->declarators[0]->name->str == SYNCASENAME_(var_float_3p25f) );
	BOOST_CHECK( SYNCASE_(var_float_3p25f)->type_info->tycode == SYNCASE_(btc_float) );
	BOOST_CHECK( SYNCASE_(exprinit_cexpr_3p25f) );
	BOOST_CHECK( SYNCASE_(exprinit_cexpr_3p25f)->node_class() == node_ids::expression_initializer );
	BOOST_CHECK( SYNCASE_(var_float_3p25f)->declarators[0]->init == SYNCASE_(exprinit_cexpr_3p25f) );
	BOOST_CHECK( SYNCASE_(exprinit_cexpr_3p25f)->init_expr == SYNCASE_( cexpr_3p25f ) );
	BOOST_CHECK( SYNCASE_(cexpr_3p25f)->node_class() == node_ids::constant_expression );
	BOOST_REQUIRE( SYNCASE_(var_3_float) );
	BOOST_REQUIRE( SYNCASE_(var_3_float)->declarators.size() == 3 );
	BOOST_REQUIRE( SYNCASE_(var_3_0) );
	BOOST_CHECK( SYNCASE_(var_3_float)->declarators[0] == SYNCASE_(var_3_0) );
	BOOST_CHECK( SYNCASE_(var_3_0)->node_class() == node_ids::declarator );
	BOOST_CHECK( SYNCASE_(var_3_0)->name->str == SYNCASENAME_(var_3_0) );
	BOOST_CHECK( ! SYNCASE_(var_3_0)->init );
	BOOST_REQUIRE( SYNCASE_(var_3_1_with_init) );
	BOOST_CHECK( SYNCASE_(var_3_1_with_init)->init );
	BOOST_CHECK( SYNCASE_(var_3_1_with_init)->init->as_handle<expression_initializer>()->init_expr = SYNCASE_(cexpr_3p25f) );
	BOOST_REQUIRE( SYNCASE_(var_3_2_with_nullinit) );
	BOOST_CHECK( SYNCASE_(var_3_2_with_nullinit) == SYNCASE_(var_3_float)->declarators[2] );
	BOOST_CHECK( SYNCASE_(var_3_2_with_nullinit)->init->node_class() == node_ids::member_initializer );
	BOOST_CHECK( SYNCASE_(var_3_2_with_nullinit)->init->as_handle<member_initializer>()->sub_inits.empty() );
	BOOST_CHECK( SYNCASE_(prog_for_syntax_test)->decls[1] == SYNCASE_(fn1_sem) );
	BOOST_CHECK( SYNCASE_(fn1_sem)->node_class() == node_ids::function_type );
	BOOST_CHECK( SYNCASE_(fn1_sem)->name->str == SYNCASENAME_(fn1_sem) );
	BOOST_CHECK( SYNCASE_(fn1_sem)->retval_type );
	BOOST_CHECK( SYNCASE_(fn1_sem)->retval_type == SYNCASE_(type_float) );
	BOOST_CHECK( SYNCASE_(fn1_sem)->params.size() == 2 );
	BOOST_CHECK( SYNCASE_(fn1_sem)->params[0] == SYNCASE_(par0_0_fn1) );
	BOOST_CHECK( SYNCASE_(par0_0_fn1)->name->str == SYNCASENAME_(par0_0_fn1) );
	BOOST_CHECK( SYNCASE_(par0_0_fn1)->param_type == SYNCASE_(type_uint64) );
	BOOST_CHECK( SYNCASE_(fn1_sem)->params[1] == SYNCASE_(par1_1_fn1) );
	BOOST_CHECK( SYNCASE_(par1_1_fn1)->name->str == SYNCASENAME_(par1_1_fn1) );
	BOOST_CHECK( SYNCASE_(fn1_sem)->body == SYNCASE_(fn1_body) );
	BOOST_CHECK( SYNCASE_(fn1_body) );
	BOOST_CHECK( SYNCASE_(fn1_body)->stmts.size() == 2 );
	BOOST_CHECK( SYNCASE_(fn1_body)->stmts[1]->node_class() == node_ids::expression_statement );

	BOOST_CHECK( SYNCASE_(prog_for_syntax_test)->decls[2] == SYNCASE_(tdef0_double2x4) );
	BOOST_CHECK( SYNCASE_(tdef0_double2x4) );
	BOOST_CHECK( SYNCASE_(tdef0_double2x4)->node_class() == node_ids::typedef_definition );
	BOOST_CHECK( SYNCASE_(tdef0_double2x4)->name->str == SYNCASENAME_(tdef0_double2x4) );
	BOOST_CHECK( SYNCASE_(tdef0_double2x4)->type_info == SYNCASE_(type_double2x4) );
}

BOOST_AUTO_TEST_CASE( type_combinator_test )
{
	using ::sasl::syntax_tree::dprog_combinator;
	using ::sasl::syntax_tree::dtype_combinator;
	using ::sasl::syntax_tree::dvar_combinator;

	using ::sasl::syntax_tree::alias_type;
	using ::sasl::syntax_tree::array_type;
	using ::sasl::syntax_tree::expression_initializer;
	using ::sasl::syntax_tree::member_initializer;
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::struct_type;
	using ::sasl::syntax_tree::tynode;
	using ::sasl::syntax_tree::variable_declaration;

	std::string var0_name( "var0_name" );
	std::string struct_name( "struct0_name" );

	boost::shared_ptr<struct program> prog;
	boost::shared_ptr<struct variable_declaration> vardecl1;
	dprog_combinator prog_comb("hello");

	boost::shared_ptr<variable_declaration> fltvar;
	boost::shared_ptr<tynode> flt;
	boost::shared_ptr<member_initializer> meminit0, meminit1;
	boost::shared_ptr<expression_initializer>
		exprinit0, exprinit1, exprinit2, exprinit3;
	{
		dvar_combinator var_comb( NULL );
		var_comb
				.dtype().dbuiltin( builtin_types::_float ).end(flt)
				.dname("What's")
					.dinit_list()
						.dinit_expr().dconstant2( (int32_t)2 ).end(exprinit0)
						.dinit_list()
							.dinit_expr().dvarexpr("expr1").end(exprinit1)
							.dinit_expr().dvarexpr("expr2").end(exprinit2)
						.end( meminit0 )
						.dinit_expr().dvarexpr( "expr0" ).end(exprinit3)
					.end( meminit1 )
				.end()
		.end( fltvar );

		BOOST_CHECK( flt );
		BOOST_CHECK( flt->node_class() == node_ids::builtin_type );
		BOOST_CHECK( flt->tycode == builtin_types::_float );

		BOOST_CHECK( fltvar );
		BOOST_CHECK( fltvar->type_info == flt );
		BOOST_CHECK( fltvar->node_class() == node_ids::variable_declaration );
		BOOST_REQUIRE( fltvar->declarators.size() == 1 );
		BOOST_CHECK( fltvar->declarators[0]->name->str == "What's" );
		BOOST_CHECK( fltvar->declarators[0]->init == meminit1 );
		BOOST_CHECK( meminit1 );
		BOOST_CHECK( meminit1->node_class() == node_ids::member_initializer );
		BOOST_CHECK( meminit1->sub_inits.size() == 3 );
		BOOST_CHECK( meminit1->sub_inits[0] == exprinit0 );
		BOOST_CHECK( exprinit0 && exprinit0->node_class() == node_ids::expression_initializer );
		BOOST_CHECK( meminit1->sub_inits[2] == exprinit3 );
		BOOST_CHECK( exprinit3 && exprinit3->node_class() == node_ids::expression_initializer );

		BOOST_CHECK( meminit1->sub_inits[1] == meminit0 );
		BOOST_CHECK( meminit0 && meminit0->node_class() == node_ids::member_initializer );
		BOOST_CHECK( meminit0->sub_inits.size() == 2 );
		BOOST_CHECK( meminit0->sub_inits[0] == exprinit1 );
		BOOST_CHECK( meminit0->sub_inits[1] == exprinit2 );
		BOOST_CHECK( exprinit2 && exprinit3 );
	}

	boost::shared_ptr<array_type> arrtype;
	{
		dtype_combinator type_comb( NULL );
		type_comb
			.dnode( flt )
			.darray().end()
			.darray().dconstant2( (int32_t)186 ).end()
		.end(arrtype);

		BOOST_CHECK( arrtype );
		BOOST_CHECK( arrtype->node_class() == node_ids::array_type );
		BOOST_CHECK( arrtype->elem_type->tycode == builtin_types::_float );
		BOOST_CHECK( arrtype->array_lens.size() == 2);
		BOOST_CHECK( !arrtype->array_lens[0] );
		BOOST_CHECK( arrtype->array_lens[1]->node_class() == node_ids::constant_expression );
	}

	boost::shared_ptr<struct_type> stype;
	{
		boost::shared_ptr<variable_declaration> member0, member1;
		dprog_combinator prog_comb( std::string("Hello") );
		prog_comb.dstruct("struct_name")
			.dmember()
				.dtype().dnode(flt).end()
				.dname("struct_member_a").end()
			.end( member0 )
			.dmember()
				.dtype().dnode(arrtype).end()
				.dname("struct_member_b").end()
			.end( member1 )
		.end(stype).end();

		BOOST_CHECK( stype );
		BOOST_CHECK( stype->node_class() == node_ids::struct_type );
		BOOST_CHECK( stype->name->str == "struct_name" );
		BOOST_CHECK( stype->decls.size() == 2 );
		BOOST_CHECK( stype->decls[0] == member0 );
		BOOST_CHECK( stype->decls[1] == member1 );
		BOOST_CHECK( member0->declarators[0]->name->str == "struct_member_a" );
		BOOST_CHECK( member0->type_info == flt );
		BOOST_CHECK( member1->declarators[0]->name->str == "struct_member_b" );
		BOOST_CHECK( member1->type_info == arrtype );
	}

	boost::shared_ptr<tynode>
		var0type, var1type, var2type;
	boost::shared_ptr<alias_type> var3type;
	boost::shared_ptr<array_type> var4type;
	prog_comb
		.dvar()
			.dtype().dvec( builtin_types::_uint64, 2 ).end( var1type )
			.dname(var0_name).end()
		.end()
		.dvar()
			.dtype().dmat( builtin_types::_double, 4, 3 ).end( var2type )
			.dname(var0_name).end()
		.end()
		.dvar()
			.dtype().dalias( struct_name ).dtypequal( type_qualifiers::_uniform ).end( var3type )
			.dname(var0_name).end()
		.end()
	.end( prog );

	BOOST_CHECK( var1type && var1type->tycode == vector_of(builtin_types::_uint64, 2) );
	BOOST_CHECK( var2type && var2type->tycode == matrix_of(builtin_types::_double, 4, 3) );
	BOOST_CHECK( var2type->node_class() == node_ids::builtin_type );
	BOOST_CHECK( !var2type->is_uniform() );
	BOOST_CHECK( var3type && var3type->alias->str == struct_name );
	BOOST_CHECK( var3type->is_uniform() );
	BOOST_CHECK( var3type->node_class() == node_ids::alias_type );

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
			.dconstant2( 0.25868f )
		.end( expr_c );
		BOOST_CHECK( expr_c->node_class() == node_ids::constant_expression );
		BOOST_CHECK( expr_c->ctype == literal_classifications::real );
		BOOST_CHECK( expr_c->value_tok->str == boost::lexical_cast<std::string>(0.25868f) + std::string("f") );
	}

	boost::shared_ptr<constant_expression> cexpr;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dconstant2( true )
		.end( cexpr );
		BOOST_CHECK( cexpr->ctype == literal_classifications::boolean );
		BOOST_CHECK( cexpr->value_tok->str == boost::lexical_cast<std::string>(true) );
	}

	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dconstant2( (uint16_t)107 )
			.end( cexpr );
		BOOST_CHECK( cexpr->ctype == literal_classifications::integer );
		BOOST_CHECK( cexpr->value_tok->str == boost::lexical_cast<std::string>(107) + std::string("u") );
	}

	boost::shared_ptr<variable_expression> varexpr;
	{
		std::string var_name("var0_name");
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dvarexpr( var_name )
		.end(varexpr);
		BOOST_CHECK( varexpr && varexpr->node_class() == node_ids::variable_expression );
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
		BOOST_CHECK( unaryexpr->node_class() == node_ids::unary_expression );
		BOOST_CHECK( unaryexpr->op == operators::negative );
		BOOST_CHECK( unaryexpr->expr == varexpr );
	}

	boost::shared_ptr<cast_expression> castexpr;
	{
		dexpr_combinator expr_comb(NULL);
		expr_comb
			.dcast()
				.dtype()
					.dbuiltin( builtin_types::_float )
				.end()
				.dexpr()
					.dnode( varexpr )
				.end()
			.end()
		.end(castexpr);
		BOOST_CHECK( castexpr );
		BOOST_CHECK( castexpr->node_class() == node_ids::cast_expression );
		BOOST_CHECK( castexpr->casted_type->tycode == builtin_types::_float );
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
		BOOST_CHECK( binexpr->node_class() == node_ids::binary_expression );
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
		BOOST_CHECK( branchexpr->node_class() == node_ids::cond_expression );
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
		BOOST_CHECK( mem1expr->node_class() == node_ids::member_expression );
		BOOST_CHECK( mem1expr->member->str == m1 );
		BOOST_CHECK( mem1expr->expr->node_class() == node_ids::member_expression );

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
		BOOST_CHECK( callexpr->node_class() == node_ids::call_expression );
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
		BOOST_CHECK( indexexpr1->node_class() == node_ids::index_expression );
		BOOST_CHECK( indexexpr1->index_expr == mem0expr );
		BOOST_CHECK( indexexpr1->expr == indexexpr0 );
		BOOST_CHECK( indexexpr0 );
		BOOST_CHECK( indexexpr0->node_class() == node_ids::index_expression );
		BOOST_CHECK( indexexpr0->index_expr == mem1expr );
		BOOST_CHECK( indexexpr0->expr == callexpr );
	}
}

BOOST_AUTO_TEST_CASE( stmt_combinator_test ){
	using ::sasl::syntax_tree::dvar_combinator;
	using ::sasl::syntax_tree::dstatements_combinator;

	using ::sasl::syntax_tree::variable_declaration;
	using ::sasl::syntax_tree::variable_expression;
	using ::sasl::syntax_tree::constant_expression;

	using ::sasl::syntax_tree::case_label;
	using ::sasl::syntax_tree::compound_statement;
	using ::sasl::syntax_tree::declaration_statement;
	using ::sasl::syntax_tree::dowhile_statement;
	using ::sasl::syntax_tree::expression_statement;
	using ::sasl::syntax_tree::for_statement;
	using ::sasl::syntax_tree::if_statement;
	using ::sasl::syntax_tree::jump_statement;
	using ::sasl::syntax_tree::switch_statement;
	using ::sasl::syntax_tree::while_statement;

	boost::shared_ptr<expression_statement> exprstmt;
	boost::shared_ptr<declaration_statement> varstmt;
	boost::shared_ptr<compound_statement> stmts;
	{
		boost::shared_ptr<variable_declaration> vardecl;
		dvar_combinator( NULL )
			.dtype().dbuiltin( builtin_types::_float ).end()
			.dname("var0").end()
		.end(vardecl);
		BOOST_CHECK( vardecl );

		dstatements_combinator( NULL )
			.dvarstmt().dnode(vardecl).end(varstmt)
			.dexprstmt().dconstant2( 1.0f ).end(exprstmt)
			.dstmts().end()
		.end( stmts );
		BOOST_CHECK( stmts );
		BOOST_CHECK( stmts->node_class() == node_ids::compound_statement );
		BOOST_CHECK( stmts->stmts.size() == 3 );
		BOOST_CHECK( stmts->stmts[0] == varstmt);
		BOOST_CHECK( varstmt->node_class() == node_ids::declaration_statement );
		BOOST_CHECK( varstmt->decl == vardecl );
		BOOST_CHECK( stmts->stmts[1] == exprstmt );
		BOOST_CHECK( exprstmt->node_class() == node_ids::expression_statement );
		BOOST_CHECK( exprstmt->expr );
		BOOST_CHECK( exprstmt->expr->node_class() == node_ids::constant_expression );
		BOOST_CHECK( stmts->stmts[2] );
		BOOST_CHECK( stmts->stmts[2]->node_class() == node_ids::compound_statement );
	}

	boost::shared_ptr<compound_statement> stmts2;
	boost::shared_ptr<if_statement> ifstmt;
	{
		boost::shared_ptr<constant_expression> condexpr;
		boost::shared_ptr<compound_statement> yesstmts;
		boost::shared_ptr<compound_statement> elsestmts;
		dstatements_combinator(NULL)
			.dif()
				.dcond().dconstant2( 1.0f ).end( condexpr )
				.dthen().dvarstmt().dnode(varstmt).end().end(yesstmts)
				.delse().dnode(stmts).end(elsestmts)
			.end(ifstmt)
		.end(stmts2)
		;

		BOOST_CHECK( stmts2 );
		BOOST_CHECK( stmts2->stmts.size() == 1 );
		BOOST_CHECK( ifstmt );
		BOOST_CHECK( stmts2->stmts[0] == ifstmt );
		BOOST_CHECK( ifstmt->node_class() == node_ids::if_statement );
		BOOST_CHECK( ifstmt->cond == condexpr );
		BOOST_CHECK( condexpr->value_tok->str == boost::lexical_cast<std::string>(1.0f) + std::string("f") );
		BOOST_CHECK( ifstmt->yes_stmt == yesstmts );
		BOOST_CHECK( yesstmts->stmts[0] == varstmt );
		BOOST_CHECK( ifstmt->no_stmt == elsestmts );
		BOOST_CHECK( elsestmts == stmts );
	}

	boost::shared_ptr<dowhile_statement> dwstmt;
	boost::shared_ptr<compound_statement> stmts3;
	{
		boost::shared_ptr<constant_expression> condexpr;
		dstatements_combinator(NULL)
			.ddowhile()
				.ddo().dnode(stmts).end(stmts3)
				.dwhile().dconstant2( 1.0f ).end( condexpr )
			.end(dwstmt)
		.end();

		BOOST_CHECK( dwstmt );
		BOOST_CHECK( dwstmt->node_class() == node_ids::dowhile_statement );
		BOOST_CHECK( dwstmt->cond == condexpr );
		BOOST_CHECK( condexpr->value_tok->str == boost::lexical_cast<std::string>(1.0f) + std::string("f") );
		BOOST_CHECK( dwstmt->body = stmts3 );
		BOOST_CHECK( stmts3 == stmts );
	}

	boost::shared_ptr<while_statement> whilestmt;
	boost::shared_ptr<compound_statement> stmts4;
	{
		boost::shared_ptr<constant_expression> condexpr;
		dstatements_combinator(NULL)
			.dwhiledo()
				.dwhile().dconstant2( 1.0f ).end( condexpr )
				.ddo().dnode(stmts).end(stmts4)
			.end(whilestmt)
		.end();

		BOOST_CHECK( whilestmt );
		BOOST_CHECK( whilestmt->node_class() == node_ids::while_statement );
		BOOST_CHECK( whilestmt->cond == condexpr );
		BOOST_CHECK( condexpr->value_tok->str == boost::lexical_cast<std::string>(1.0f) + std::string("f") );
		BOOST_CHECK( whilestmt->body = stmts4 );
		BOOST_CHECK( stmts4 == stmts );
	}

	boost::shared_ptr<switch_statement> switchstmt;
	boost::shared_ptr<compound_statement> stmts5, stmts6;
	{
		boost::shared_ptr<case_label> clbl;
		boost::shared_ptr<case_label> clbl2;
		boost::shared_ptr<variable_expression> expr;
		dstatements_combinator( NULL )
			.dswitch()
				.dexpr().dvarexpr( "hello" ).end( expr )
				.dbody()
					.dcase().dconstant2( 1.0f ).end( clbl )
					.dcase().dconstant2( 2.0f ).end( clbl2 )
					.dwhiledo().dnode(whilestmt).end()
					.dif().dnode( ifstmt ).end()
					.ddefault()
					.dvarstmt().dnode( varstmt ).end()
				.end( stmts5 )
			.end(switchstmt)
		.end( stmts6 );

		BOOST_CHECK( stmts6 );
		BOOST_CHECK( stmts6->stmts[0] == switchstmt );
		BOOST_CHECK( switchstmt->node_class() == node_ids::switch_statement );
		BOOST_CHECK( switchstmt->cond == expr );
		BOOST_CHECK( expr->var_name->str == std::string("hello") );
		BOOST_CHECK( switchstmt->stmts == stmts5 );
		BOOST_CHECK( stmts5->stmts.size() == 3 );
		BOOST_CHECK( stmts5->stmts[0] == whilestmt );
		BOOST_CHECK( stmts5->stmts[1] == ifstmt );
		BOOST_CHECK( stmts5->stmts[2] == varstmt );
		BOOST_CHECK( whilestmt->labels.size() == 2 );
		BOOST_CHECK( whilestmt->labels[0] == clbl );
		BOOST_CHECK( whilestmt->labels[1] == clbl2 );
		BOOST_CHECK( clbl->node_class() == node_ids::case_label );
		BOOST_CHECK( clbl->expr->node_class() == node_ids::constant_expression );
		BOOST_CHECK( ifstmt->labels.size() == 0 );
		BOOST_CHECK( varstmt->labels.size() == 1 );
		BOOST_CHECK( !boost::shared_polymorphic_cast<case_label>(varstmt->labels[0])->expr );
	}

	boost::shared_ptr<compound_statement> stmts7;
	boost::shared_ptr<jump_statement> jmpstmt;
	{
		boost::shared_ptr<variable_expression> expr;
		dstatements_combinator( NULL )
			.dbreak()
			.dcontinue()
			.dreturn_expr().dvarexpr( "hello" ).end(jmpstmt)
			.dreturn_void()
		.end(stmts7);

		BOOST_CHECK( stmts7 );
		BOOST_CHECK( stmts7->stmts.size() == 4 );
		BOOST_CHECK( stmts7->stmts[0]->node_class() == node_ids::jump_statement );
		BOOST_CHECK( boost::shared_polymorphic_cast<jump_statement>(stmts7->stmts[0])->code == jump_mode::_break );
		BOOST_CHECK( boost::shared_polymorphic_cast<jump_statement>(stmts7->stmts[1])->code == jump_mode::_continue );
		BOOST_CHECK( stmts7->stmts[2] == jmpstmt );
		BOOST_CHECK( jmpstmt->code == jump_mode::_return );
		BOOST_CHECK( jmpstmt->jump_expr );
		BOOST_CHECK( jmpstmt->jump_expr->node_class() == node_ids::variable_expression );
		BOOST_CHECK( boost::shared_polymorphic_cast<jump_statement>(stmts7->stmts[3])->code == jump_mode::_return );
		BOOST_CHECK( ! boost::shared_polymorphic_cast<jump_statement>(stmts7->stmts[3])->jump_expr );
	}

	boost::shared_ptr<compound_statement> stmts8;
	boost::shared_ptr<for_statement> forstmt;
	{
		dstatements_combinator( NULL )
			.dfor()
				.dinit_expr().dnode( exprstmt ).end()
				.dcond().dvarexpr( "hello" ).end()
				.diter().dconstant2( 1.0f ).end()
				.dbody().dnode( stmts7 ).end()
			.end(forstmt)
		.end( stmts8 );
	}

	BOOST_CHECK( stmts8 );
	BOOST_CHECK( stmts8->stmts.size() == 1 );
	BOOST_CHECK( stmts8->stmts[0] == forstmt );
	BOOST_CHECK( forstmt );
	BOOST_CHECK( forstmt->init );
	BOOST_CHECK( forstmt->init == exprstmt );
	BOOST_CHECK( forstmt->cond->node_class() == node_ids::variable_expression );
	BOOST_CHECK( forstmt->iter->node_class() == node_ids::constant_expression );
	BOOST_CHECK( forstmt->body == stmts7 );
}
BOOST_AUTO_TEST_SUITE_END();
