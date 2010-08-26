#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/visitor.h>

BEGIN_NS_SASL_SYNTAX_TREE();

initializer::initializer( syntax_node_types type_id, boost::shared_ptr<token_attr> tok )
	: node( type_id, tok ){
}

expression_initializer::expression_initializer( boost::shared_ptr<token_attr> tok )
	: initializer( syntax_node_types::expression_initializer, tok ) {
}

void expression_initializer::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

member_initializer::member_initializer( boost::shared_ptr< token_attr > tok )
	: initializer( syntax_node_types::member_initializer, tok ) { }

void member_initializer::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

declaration::declaration( syntax_node_types type_id, boost::shared_ptr<token_attr> tok )
	: node( type_id, tok )
{
}

variable_declaration::variable_declaration( boost::shared_ptr<token_attr> tok )
	: declaration(syntax_node_types::variable_declaration, tok ) {
}

void variable_declaration::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

type_definition::type_definition( boost::shared_ptr<token_attr> tok )
	: declaration( syntax_node_types::typedef_definition, tok ){
}

void type_definition::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

type_specifier::type_specifier( syntax_node_types type_id, boost::shared_ptr<token_attr> tok )
	: declaration( type_id, tok ),
	value_typecode( buildin_type_code::none),
	qual( type_qualifiers::none )
{ }

bool type_specifier::is_buildin() const{
	return node_class() == syntax_node_types::buildin_type;
}

bool type_specifier::is_uniform() const
{
	return qual.included( type_qualifiers::_uniform );
}

buildin_type::buildin_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::buildin_type, tok ){ }

void buildin_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

array_type::array_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::array_type, tok ) { }

void array_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

struct_type::struct_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::struct_type, tok ) {}

void struct_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

parameter::parameter( boost::shared_ptr<token_attr> tok )
	: declaration( syntax_node_types::parameter, tok ) {
}

void parameter::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

function_type::function_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::function_type, tok ), is_declaration(true)
{
}

void function_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

function_type& function_type::p( boost::shared_ptr<variable_declaration> par ){
	if (par){
		return p( par->type_info, par->name, par->init );
	}else{
		return *this;
	}
}

function_type& function_type::s( boost::shared_ptr<statement> stmt ){
	if ( stmt ){
		stmts.push_back( stmt );
		is_declaration = false;
	}
	return *this;
}

buildin_type_code btc_helper::vector_of( buildin_type_code scalar_tc, size_t dim )
{
	assert( is_scalar(scalar_tc) );
	assert( 1 <= dim && dim <= 4 );

	buildin_type_code ret( scalar_tc | buildin_type_code::_vector );
	ret.from_value( ret.to_value() | ( dim << ret._dim0_field_shift.to_value() ) );
	return ret;
}

buildin_type_code btc_helper::matrix_of( buildin_type_code scalar_tc, size_t dim0, size_t dim1 )
{
	assert( is_scalar(scalar_tc) );
	assert( 1 <= dim0 && dim0 <= 4 && 1 <= dim1 && dim1 <= 4 );

	buildin_type_code ret( scalar_tc | buildin_type_code::_matrix );
	ret.from_value(
		ret.to_value()
		| ( dim0 << ret._dim0_field_shift.to_value() )
		| ( dim1 << ret._dim1_field_shift.to_value() )
		);

	return ret;
}

size_t btc_helper::dim0_len( buildin_type_code btc )
{
	if( is_scalar(btc) )
	{
		return 0;
	}
	return (size_t)
		(
		(btc & buildin_type_code::_dim0_mask).to_value()
		>> buildin_type_code::_dim0_field_shift.to_value()
		);
}

size_t btc_helper::dim1_len( buildin_type_code btc )
{
	if( is_scalar(btc) || is_vector(btc) )
	{
		return 0;
	}
	return (size_t)
		(
		(btc & buildin_type_code::_dim1_mask).to_value()
		>> buildin_type_code::_dim1_field_shift.to_value()
		);
}

bool btc_helper::is_scalar( buildin_type_code btc )
{
	return ( btc & buildin_type_code::_dimension_mask ) == buildin_type_code::_scalar;
}

bool btc_helper::is_vector( buildin_type_code btc )
{
	return ( btc & buildin_type_code::_dimension_mask ) == buildin_type_code::_vector;
}

bool btc_helper::is_matrix( buildin_type_code btc )
{
	return ( btc & buildin_type_code::_dimension_mask ) == buildin_type_code::_matrix;
}

buildin_type_code btc_helper::scalar_of( buildin_type_code btc )
{
	return btc & buildin_type_code::_scalar_type_mask;
}

END_NS_SASL_SYNTAX_TREE();