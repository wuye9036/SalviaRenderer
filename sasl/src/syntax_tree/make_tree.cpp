#include <sasl/include/syntax_tree/make_tree.h>

#include <sasl/enums/buildin_type_code.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <boost/static_assert.hpp>
//#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_same.hpp>

BEGIN_NS_SASL_SYNTAX_TREE();

using ::sasl::common::token_attr;

extern literal_constant_types type_codes[] =
{
	literal_constant_types::boolean,
	literal_constant_types::integer,
	literal_constant_types::integer,
	literal_constant_types::integer,
	literal_constant_types::integer,
	literal_constant_types::integer,
	literal_constant_types::integer,
	literal_constant_types::integer,
	literal_constant_types::integer,
	literal_constant_types::real,
	literal_constant_types::real
};

SASL_TYPED_NODE_ACCESSORS_IMPL( tree_combinator, node );

/////////////////////////////////
// program combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dprog_combinator, program );

dprog_combinator::dprog_combinator( const std::string& prog_name ):
	tree_combinator(NULL)
{
	typed_node( create_node<program>( prog_name ) );
}

tree_combinator& dprog_combinator::dvar( const std::string& var_name )
{
	var_comb = boost::shared_ptr<dvar_combinator>( new dvar_combinator(*this) );

	boost::shared_ptr<variable_declaration> vardecl = create_node<variable_declaration>( token_attr::null() );
	typed_node()->decls.push_back( vardecl );
	var_comb->typed_node( vardecl );
	var_comb->dname( var_name );

	return *var_comb;
}
/////////////////////////////////
// type combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dtype_combinator, type_specifier );
dtype_combinator::dtype_combinator( tree_combinator& parent )
: tree_combinator( boost::addressof(parent) )
{
}

tree_combinator& dtype_combinator::dbuildin( buildin_type_code btc )
{
	if( cur_node ){
		return default_proc();
	}
	
	typed_node( create_node<buildin_type>(token_attr::null()) )->value_typecode = btc;
	return *this;
}

tree_combinator& dtype_combinator::dvec( buildin_type_code comp_btc, size_t size )
{
	if ( cur_node ){
		return default_proc();
	}

	typed_node( create_node<buildin_type>(token_attr::null()) );
	typed_node()->value_typecode = btc_helper::vector_of( comp_btc, size );
}

/////////////////////////////////////
// variable combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dvar_combinator, variable_declaration );

dvar_combinator::dvar_combinator( tree_combinator& parent )
: tree_combinator( boost::addressof(parent) )
{
}

tree_combinator& dvar_combinator::dname( const std::string& name )
{
	typed_node()->name = token_attr::from_string(name);
	return *this;
}

tree_combinator& dvar_combinator::dtype()
{
	type_comb = boost::shared_ptr<dtype_combinator>( new dtype_combinator(*this) );
	e_state = e_type;
	return *type_comb;
}

void dvar_combinator::child_ended()
{
	switch( e_state )
	{
	case e_none:
		default_proc();
		break;
	case e_type:
		typed_node()->type_info = type_comb->typed_node();
		break;
	case e_init:
		// typed_node()->init = init_comb->typed_node();
		break;
	}
	e_state = e_none;
}



END_NS_SASL_SYNTAX_TREE();

struct empty_type{
	static boost::shared_ptr<empty_type> null(){
		return boost::shared_ptr<empty_type>();
	}
};

struct no_matched{
};
// do nothing...
no_matched make_tree( ... ){
	return no_matched();
}

template<typename U, typename T>
bool is_same_type( T, EFLIB_ENABLE_IF_PRED2( is_same, U, T, 0 ) ){
	return true;
}

template<typename U, typename T>
bool is_same_type( T, EFLIB_DISABLE_IF_PRED2( is_same, U, T, 0 ) ){
	return false;
}

template <typename T>
T& null_instance(){
	return *((T*)NULL);
}



//BOOST_AUTO_TEST_SUITE( tree_maker )
//
//BOOST_AUTO_TEST_CASE( make_tree_test ){
//	::std::string litstr("_test_case_only_");
//	BOOST_CHECK_EQUAL( make_tree( litstr )->lit, litstr );
//
//	buildin_type_code dbltc( buildin_type_code::_double );
//	boost::shared_ptr<sasl::syntax_tree::buildin_type> dbltype = make_tree( dbltc );
//	BOOST_CHECK( dbltype->value_typecode == dbltc );
//	
//	boost::shared_ptr<sasl::syntax_tree::variable_declaration> dblvar = make_tree( dbltype, litstr );
//	BOOST_CHECK( !dblvar->init );
//	BOOST_CHECK( dblvar->name->lit == litstr );
//	BOOST_CHECK( dblvar->type_info->value_typecode == dbltc );	
//
//	BOOST_CHECK( is_same_type<no_matched>( make_tree( empty_type::null(), litstr ) ) );
//}
//
//BOOST_AUTO_TEST_SUITE_END()