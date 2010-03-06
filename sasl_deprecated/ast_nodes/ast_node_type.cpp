#include "ast_node_type.h"
#include "ast_node_type_qualifier.h"

#include <boost/foreach.hpp>
#include <algorithm>

using namespace std;

/////////////////////
//  ast_node_type  //
/////////////////////
std::vector<type_qualifiers>& ast_node_type::get_type_qualifiers(){
	return quals_;
}

const std::vector<type_qualifiers>& ast_node_type::get_type_qualifiers() const{
	return quals_;
}

bool ast_node_type::has_qualifier( const type_qualifiers& qual ) const{
	return find( quals_.begin(), quals_.end(), qual ) != quals_.end();
}

ast_node_type::ast_node_type( const std::vector<type_qualifiers>& quals, const ast_node_types& node_type )
:ast_node(node_type), quals_(quals){
}

ast_node_type::ast_node_type(const ast_node_types& node_type)
:ast_node(node_type)
{}


/////////////////////////
//  ast_node_function  //
/////////////////////////
h_ast_node_function ast_node_function::create( h_ast_node_type ret_type, const std::vector<h_ast_node_variable_declaration>& params, h_ast_node_statement body ){
	return h_ast_node_function( new ast_node_function(ret_type, params, body ));
}

std::string ast_node_function::get_typename() const{
	return string();
}

void ast_node_function::visit( ast_visitor* visitor ){
}

void ast_node_function::release(){
}

ast_node_function::ast_node_function( h_ast_node_type ret_type, const std::vector<h_ast_node_variable_declaration>& params, h_ast_node_statement body )
: ret_type_(ret_type), params_(params), body_(body){
}

h_ast_node_type ast_node_function::get_return_type() const{
	return ret_type_;
}

const std::vector< h_ast_node_variable_declaration >& ast_node_function::get_parameters() const{
	return params_;
}

h_ast_node_statement ast_node_function::get_function_body() const{
	return body_;
}
//////////////////////
//  ast_node_array  //
//////////////////////
h_ast_node_array ast_node_array::create( h_ast_node_type elem_type, const std::vector<h_ast_node_expression>& size_exprs ){
	return h_ast_node_array( new ast_node_array(elem_type, size_exprs) );
}

std::string ast_node_array::get_typename() const{
	return string();
}

void ast_node_array::visit( ast_visitor* visitor ){
}

void ast_node_array::release(){
}

ast_node_array::ast_node_array( h_ast_node_type elem_type, const std::vector<h_ast_node_expression>& size_exprs )
: ast_node_type(ast_node_types::array_type), elem_type_(elem_type), size_exprs_(size_exprs){
}

std::vector<h_ast_node_expression>& ast_node_array::get_size_expressions(){
	return size_exprs_;
}

const std::vector<h_ast_node_expression>& ast_node_array::get_size_expressions() const{
	return size_exprs_;
}

h_ast_node_type ast_node_array::get_element_type() const{
	return elem_type_;
}

//////////////////////
// ast_node_struct  //
//////////////////////

h_ast_node_struct ast_node_struct::create( h_ast_node_identifier ident, const std::vector<h_ast_node_declaration>& members ){
	return h_ast_node_struct( new ast_node_struct( ident, members ) );
}

std::string ast_node_struct::get_typename() const{
	return string();
}

void ast_node_struct::visit( ast_visitor* visitor ){
}

void ast_node_struct::release(){
}

const std::vector<h_ast_node_declaration>& ast_node_struct::get_members() const{
	return members_;
}

h_ast_node_identifier ast_node_struct::get_identifier() const{
	return ident_;
}

ast_node_struct::ast_node_struct( h_ast_node_identifier ident, const std::vector<h_ast_node_declaration>& members )
: ident_(ident), members_(members){
}

//////////////////////////////
// ast_node_identifier_type //
//////////////////////////////
h_ast_node_identifier_type ast_node_identifier_type::create(h_ast_node_identifier ident){
	return h_ast_node_identifier_type( new ast_node_identifier_type(ident) );
}

string ast_node_identifier_type::get_typename() const{
	return string();
}

void ast_node_identifier_type::visit( ast_visitor* visitor ){
	visitor->enter_node( this );
	visitor->leave_node( this );
}

void ast_node_identifier_type::release(){
	return;
}

h_ast_node_identifier ast_node_identifier_type::get_identifier() const{
	return ident_;
}

ast_node_identifier_type::ast_node_identifier_type( h_ast_node_identifier ident ): ident_(ident){
}

