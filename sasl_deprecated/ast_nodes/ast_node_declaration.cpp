#include "ast_node_declaration.h"
#include "ast_node_type.h"

#include <boost/foreach.hpp>

using namespace std;

/////////////////////////////////////
//  ast_node_variable_declaration  //
/////////////////////////////////////
h_ast_node_variable_declaration ast_node_variable_declaration::create( 
	h_ast_node_type var_type, 
	h_ast_node_identifier identifier, 
	h_ast_node_expression initializer ){
	return h_ast_node_variable_declaration( new ast_node_variable_declaration(var_type, identifier, initializer) );
}
std::string ast_node_variable_declaration::get_typename() const {
	return string();
}

void ast_node_variable_declaration::visit( ast_visitor* visitor ){
	visitor->enter_node( self_ );
	{
		var_type_->visit( visitor );
		identifier_->visit( visitor );
		initializer_->visit( visitor );
	}
	visitor->leave_node( self_ );
}

void ast_node_variable_declaration::release(){
	delete this;
}

h_ast_node_type ast_node_variable_declaration::get_type() const{
	return var_type_;
}

h_ast_node_identifier ast_node_variable_declaration::get_identifier() const {
	return identifier_;
}

h_ast_node_expression ast_node_variable_declaration::get_initializer() const {
	return initializer_;
}

ast_node_variable_declaration::ast_node_variable_declaration( h_ast_node_type var_type,
	h_ast_node_identifier identifier,
	h_ast_node_expression initializer )
	: var_type_(var_type), identifier_(identifier), initializer_(initializer)
{
}

//////////////////////////////////
//  ast_node_block_declaration  //
//////////////////////////////////
h_ast_node_block_declaration ast_node_block_declaration::create( const std::vector<h_ast_node_variable_declaration>& decls ){
	return h_ast_node_block_declaration( new ast_node_block_declaration(decls) );
}

std::string ast_node_block_declaration::get_typename() const{
	return string();
}

void ast_node_block_declaration::visit( ast_visitor* visitor ){
	visitor->enter_node( self_ );
	{
		BOOST_FOREACH( h_ast_node_variable_declaration decl, decls_ ){
			decl->visit( visitor );
		}
	}
	visitor->leave_node( self_ );
}

void ast_node_block_declaration::release(){
}

const std::vector<h_ast_node_variable_declaration>& ast_node_block_declaration::get_declarations() const{
	return decls_;
}

ast_node_block_declaration::ast_node_block_declaration( const std::vector<h_ast_node_variable_declaration>& decls )
:decls_(decls){
}

///////////////////////////////////////
//  ast_node_initialized_declarator  //
///////////////////////////////////////
h_ast_node_initialized_declarator ast_node_initialized_declarator::create( h_ast_node_identifier ident, h_ast_node_expression init_expr ){
	return h_ast_node_initialized_declarator( new ast_node_initialized_declarator( ident, init_expr ) );
}

h_ast_node_identifier ast_node_initialized_declarator::get_identifier() const{
	return ident_;
}

h_ast_node_expression ast_node_initialized_declarator::get_initializer() const{
	return initializer_;
}

ast_node_initialized_declarator::ast_node_initialized_declarator( h_ast_node_identifier ident, h_ast_node_expression init_expr )
:ident_(ident), initializer_(init_expr){
}

std::string ast_node_initialized_declarator::get_typename() const{
	return string();
}

void ast_node_initialized_declarator::visit( ast_visitor* visitor ){
}

void ast_node_initialized_declarator::release(){
}