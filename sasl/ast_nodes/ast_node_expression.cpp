#include "ast_node_expression.h"
#include "ast_node_type.h"

#include <boost/foreach.hpp>

using namespace std;

//////////////////////////////////
// ast_node_variable_expression //
//////////////////////////////////
h_ast_node_variable_expression ast_node_variable_expression::create(h_ast_node_identifier ident){
	return h_ast_node_variable_expression( new ast_node_variable_expression( ident ) );
}

ast_node_variable_expression::ast_node_variable_expression(h_ast_node_identifier ident)
: ident_(ident){
}

h_ast_node_identifier ast_node_variable_expression::get_identifier() const{
	return ident_;
}

string ast_node_variable_expression::get_typename() const{
	return string();
}

void ast_node_variable_expression::release(){
}

void ast_node_variable_expression::visit(ast_visitor *visitor){
	visitor->enter_node( self_ );
	visitor->leave_node( self_ );
}

///////////////////////////////
// ast_node_unary_expression //
///////////////////////////////
ast_node_unary_expression::ast_node_unary_expression(operators op, h_ast_node_expression expr)
:expr_(expr), op_(op){
}

h_ast_node_unary_expression ast_node_unary_expression::create(operators op, h_ast_node_expression expr){
	return h_ast_node_unary_expression( new ast_node_unary_expression(op, expr) );
}

h_ast_node_expression ast_node_unary_expression::get_expression() const{
	return expr_;
}

operators ast_node_unary_expression::get_op() const{
	return op_;
}

string ast_node_unary_expression::get_typename() const{
	return string();
}

void ast_node_unary_expression::release(){
}

void ast_node_unary_expression::visit(ast_visitor *visitor){
	visitor->enter_node( self_ );
	{
		expr_->visit( visitor );
	}
	visitor->leave_node( self_ );
}

//////////////////////////////
// ast_node_call_expression //
//////////////////////////////
h_ast_node_call_expression ast_node_call_expression::create( h_ast_node_expression function_expr, const std::vector<h_ast_node_expression>& parameter_exprs ){
	return h_ast_node_call_expression( new ast_node_call_expression( function_expr, parameter_exprs ) );
}

string ast_node_call_expression::get_typename() const{
	return string();
}

void ast_node_call_expression::visit( ast_visitor* visitor ){
	visitor->enter_node( self_ );
	{
		function_expr_->visit( visitor );
		BOOST_FOREACH( h_ast_node_expression param_expr, param_exprs_ ){
			param_expr->visit( visitor );
		}
	}
	visitor->leave_node( self_ );
}

void ast_node_call_expression::release(){
}

h_ast_node_expression ast_node_call_expression::get_function_expression() const{
	return function_expr_;
}

const vector<h_ast_node_expression>& ast_node_call_expression::get_parameter_expressions() const{
	return param_exprs_;
}

ast_node_call_expression::ast_node_call_expression( h_ast_node_expression function_expr, const std::vector<h_ast_node_expression>& parameter_exprs )
:function_expr_(function_expr), param_exprs_(parameter_exprs){
}

/////////////////////////////////////
// ast_node_index_expression //
/////////////////////////////////////

h_ast_node_index_expression ast_node_index_expression::create( h_ast_node_expression array_expr, h_ast_node_expression index_expr ){
	return h_ast_node_index_expression( new ast_node_index_expression( array_expr, index_expr ) );
}

string ast_node_index_expression::get_typename() const{
	return string();
}

void ast_node_index_expression::visit( ast_visitor* visitor ){
	visitor->enter_node( self_ );
	{
		array_expr_->visit( visitor );
		index_expr_->visit( visitor );
	}
	visitor->leave_node( self_ );
}

void ast_node_index_expression::release(){
}

h_ast_node_expression ast_node_index_expression::get_array_expression() const{
	return array_expr_;
}

h_ast_node_expression ast_node_index_expression::get_index_expression() const{
	return index_expr_;
}

ast_node_index_expression::ast_node_index_expression( h_ast_node_expression array_expr, h_ast_node_expression index_expr )
:array_expr_(array_expr), index_expr_(index_expr)
{}

////////////////////////////////
// ast_node_member_expression //
////////////////////////////////

h_ast_node_member_expression ast_node_member_expression::create( h_ast_node_expression obj_expr, h_ast_node_identifier ident ){
	return h_ast_node_member_expression( new ast_node_member_expression( obj_expr, ident ) );
}

std::string ast_node_member_expression::get_typename() const{
	return string();
}

void ast_node_member_expression::visit( ast_visitor* visitor ){
	visitor->enter_node( self_ );
	{
		obj_expr_->visit( visitor );
		member_identifier_->visit( visitor );
	}
	visitor->leave_node( self_ );
}

void ast_node_member_expression::release(){
}

h_ast_node_expression ast_node_member_expression::get_obj_expression() const{
	return obj_expr_;
}

h_ast_node_identifier ast_node_member_expression::get_member_identifier() const{
	return member_identifier_;
}

ast_node_member_expression::ast_node_member_expression( h_ast_node_expression obj_expr, h_ast_node_identifier member_ident )
: obj_expr_(obj_expr), member_identifier_(member_ident){
}

////////////////////////////////
// ast_node_binary_expression //
////////////////////////////////
h_ast_node_binary_expression ast_node_binary_expression::create( h_ast_node_expression lexpr, operators op, h_ast_node_expression rexpr ){
	return h_ast_node_binary_expression( new ast_node_binary_expression(lexpr, op, rexpr) );
}

std::string ast_node_binary_expression::get_typename() const{
	return string();
}

void ast_node_binary_expression::visit( ast_visitor* visitor ){
	visitor->enter_node( self_ );
	{
		left_expr_->visit( visitor );
		right_expr_->visit( visitor );
	}
	visitor->leave_node( self_ );
}

void ast_node_binary_expression::release(){
}

h_ast_node_expression ast_node_binary_expression::get_left_expressions() const{
	return left_expr_;
}

h_ast_node_expression ast_node_binary_expression::get_right_expression() const{
	return right_expr_;
}

ast_node_binary_expression::ast_node_binary_expression( h_ast_node_expression lexpr, operators op, h_ast_node_expression rexpr )
: left_expr_(lexpr), op_(op), right_expr_(rexpr){
}

///////////////////////////////////
// ast_node_condition_epxression //
///////////////////////////////////

h_ast_node_condition_expression ast_node_condition_expression::create( h_ast_node_expression cond_expr, h_ast_node_expression true_expr, h_ast_node_expression false_expr ){
	return h_ast_node_condition_expression( new ast_node_condition_expression( cond_expr, true_expr, false_expr ) );
}

string ast_node_condition_expression::get_typename() const{
	return string();
}

void ast_node_condition_expression::visit( ast_visitor* visitor ){
	visitor->enter_node( self_ );
	{
		cond_expr_->visit( visitor );
		true_expr_->visit( visitor );
		false_expr_->visit( visitor );
	}
	visitor->leave_node( self_ );
}

void ast_node_condition_expression::release(){
}

h_ast_node_expression ast_node_condition_expression::get_condition_expression() const
{
	return cond_expr_;
}

h_ast_node_expression ast_node_condition_expression::get_true_expression() const
{
	return true_expr_;
}

h_ast_node_expression ast_node_condition_expression::get_false_expression() const
{
	return false_expr_;
}

ast_node_condition_expression::ast_node_condition_expression( h_ast_node_expression cond_expr, h_ast_node_expression true_expr, h_ast_node_expression false_expr )
:cond_expr_(cond_expr), true_expr_(true_expr), false_expr_(false_expr){
}

////////////////////////////////
//  ast_node_expression_list  //
////////////////////////////////

h_ast_node_expression_list ast_node_expression_list::create( const std::vector<h_ast_node_expression>& exprs ){
	return h_ast_node_expression_list( new ast_node_expression_list(exprs) );
}

std::string ast_node_expression_list::get_typename() const{
	return string();
}

void ast_node_expression_list::visit( ast_visitor* visitor ){
	visitor->enter_node( self_ );
	{
		BOOST_FOREACH( h_ast_node_expression expr, exprs_ ){
			expr->visit( visitor );
		}
	}
	visitor->leave_node( self_ );
}

const std::vector<h_ast_node_expression>& ast_node_expression_list::get_expressions() const{
	return exprs_;
}

ast_node_expression_list::ast_node_expression_list( const std::vector<h_ast_node_expression>& exprs )
: ast_node_expression(ast_node_types::expression_list), exprs_(exprs){
}

////////////////////////////////
//  ast_node_cast_expression  //
////////////////////////////////

h_ast_node_cast_expression ast_node_cast_expression::create( h_ast_node_type type_spec, h_ast_node_expression expr )
{
	return h_ast_node_cast_expression( new ast_node_cast_expression(type_spec, expr) );
}

string ast_node_cast_expression::get_typename() const
{
	return string();
}

void ast_node_cast_expression::visit( ast_visitor* visitor )
{
	visitor->enter_node( self_ );
	{
		type_spec_->visit( visitor );
		expr_->visit( visitor );
	}
	visitor->leave_node( self_ );
}

ast_node_cast_expression::ast_node_cast_expression( h_ast_node_type type_spec, h_ast_node_expression expr )
: type_spec_(type_spec), expr_(expr){
}
