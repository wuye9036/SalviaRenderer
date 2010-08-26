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
	var_comb = boost::make_shared<dvar_combinator>(this);

	boost::shared_ptr<variable_declaration> vardecl = create_node<variable_declaration>( token_attr::null() );
	typed_node()->decls.push_back( vardecl );
	var_comb->typed_node( vardecl );
	var_comb->dname( var_name );

	return *var_comb;
}
/////////////////////////////////
// type combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dtype_combinator, type_specifier );

dtype_combinator::dtype_combinator( tree_combinator* parent )
: tree_combinator( parent ), e_state(e_none)
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
	return *this;
}

tree_combinator& dtype_combinator::dmat( buildin_type_code comp_btc, size_t s0, size_t s1 )
{
	if( cur_node ){
		return default_proc();
	}
	typed_node( create_node<buildin_type>(token_attr::null() ) );
	typed_node()->value_typecode = btc_helper::matrix_of(comp_btc, s0, s1);
	return *this;
}

tree_combinator& dtype_combinator::dalias( const std::string& alias )
{
	if( cur_node ){
		return default_proc();
	}
	typed_node( create_node<struct_type>( token_attr::null() ) );
	typed_node2<struct_type>()->name = token_attr::from_string(alias);
	return *this;
}

tree_combinator& dtype_combinator::dtypequal( type_qualifiers qual )
{
	if( !cur_node || typed_node()->qual != type_qualifiers::none )
	{
		return default_proc();
	}
	typed_node()->qual = qual;
	return *this;
}

tree_combinator& dtype_combinator::darray()
{
	if ( !cur_node ) { return default_proc(); }
	e_state = e_array;
	expr_comb = boost::make_shared<dexpr_combinator>(this);
	return *expr_comb;
}

void dtype_combinator::child_ended()
{
	if( e_state == e_array ){
		boost::shared_ptr<array_type> outter_type = create_node<array_type>( token_attr::null() );
		outter_type->array_lens.push_back( expr_comb->typed_node() );
		outter_type->elem_type = typed_node();
		typed_node( outter_type );
		e_state = e_none;
	}
}

/////////////////////////////////////
// variable combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dvar_combinator, variable_declaration );

dvar_combinator::dvar_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
}

tree_combinator& dvar_combinator::dname( const std::string& name )
{
	typed_node()->name = token_attr::from_string(name);
	return *this;
}

tree_combinator& dvar_combinator::dtype()
{
	type_comb = boost::make_shared<dtype_combinator>(this);
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


/////////////////////////////////////////////////////////////////
// expression combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dexpr_combinator, expression );

dexpr_combinator::dexpr_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
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