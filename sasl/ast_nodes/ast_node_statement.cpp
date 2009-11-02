#include "ast_node_statement.h"

using namespace std;

///////////////////////////////////
//  ast_node_compound_statement  //
///////////////////////////////////
h_ast_node_compound_statement ast_node_compound_statement::create( const std::vector<h_ast_node_statement>& stmts ){
	return h_ast_node_compound_statement( new ast_node_compound_statement(stmts) );
}

const std::vector<h_ast_node_statement>& ast_node_compound_statement::get_statements() const{
	return stmts_;
}

void ast_node_compound_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_compound_statement::get_typename() const{
	return string();
}

ast_node_compound_statement::ast_node_compound_statement( const std::vector<h_ast_node_statement>& stmts )
: stmts_(stmts){
}

//////////////////////////////////////
//  ast_node_declaration_statement  //
//////////////////////////////////////
h_ast_node_declaration_statement ast_node_declaration_statement::create( h_ast_node_declaration decl ){
	return h_ast_node_declaration_statement( new ast_node_declaration_statement(decl) );
}

void ast_node_declaration_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_declaration_statement::get_typename() const{
	return string();
}

h_ast_node_declaration ast_node_declaration_statement::get_declaration() const{
	return decl_;
}

ast_node_declaration_statement::ast_node_declaration_statement( h_ast_node_declaration decl )
: decl_(decl){
}

/////////////////////////////////////
//  ast_node_expression_statement  //
/////////////////////////////////////
h_ast_node_expression_statement ast_node_expression_statement::create( h_ast_node_expression expr ){
	return h_ast_node_expression_statement( new ast_node_expression_statement(expr) );
}

void ast_node_expression_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_expression_statement::get_typename() const{
	return string();
}

h_ast_node_expression ast_node_expression_statement::get_expression() const{
	return expr_;
}

ast_node_expression_statement::ast_node_expression_statement( h_ast_node_expression expr )
:expr_(expr){
}

/////////////////////////////
//  ast_node_if_statement  //
/////////////////////////////

h_ast_node_if_statement ast_node_if_statement::create( h_ast_node_expression cond_expr, h_ast_node_statement if_stmt, h_ast_node_statement else_stmt ){
	return h_ast_node_if_statement( new ast_node_if_statement( cond_expr, if_stmt, else_stmt) );
}

h_ast_node_expression ast_node_if_statement::get_condition_expression(){
	return cond_expr_;
}

h_ast_node_statement ast_node_if_statement::get_if_statement(){
	return if_stmt_;
}

h_ast_node_statement ast_node_if_statement::get_else_statement(){
	return else_stmt_;
}

void ast_node_if_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_if_statement::get_typename() const{
	return string();
}

ast_node_if_statement::ast_node_if_statement( h_ast_node_expression cond_expr, h_ast_node_statement if_stmt, h_ast_node_statement else_stmt )
: cond_expr_(cond_expr), if_stmt_(if_stmt), else_stmt_(else_stmt){
}

/////////////////////////////////
//  ast_node_switch_statement  //
/////////////////////////////////
h_ast_node_switch_statement ast_node_switch_statement::create( h_ast_node_expression cond_expr, const std::vector<h_ast_node_statement>& stmts ){
	return h_ast_node_switch_statement( new ast_node_switch_statement(cond_expr, stmts) );
}

void ast_node_switch_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_switch_statement::get_typename() const{
	return string();
}

h_ast_node_expression ast_node_switch_statement::get_condition_expression() const{
	return cond_expr_;
}

const std::vector<h_ast_node_statement>& ast_node_switch_statement::get_statements() const{
	return stmts_;
}

ast_node_switch_statement::ast_node_switch_statement( h_ast_node_expression cond_expr, const std::vector<h_ast_node_statement>& stmts )
: cond_expr_(cond_expr), stmts_(stmts){
}

////////////////////////////////
//  ast_node_while_statement  //
////////////////////////////////
h_ast_node_while_statement ast_node_while_statement::create( h_ast_node_expression cond_expr, h_ast_node_statement stmt ){
	return h_ast_node_while_statement( new ast_node_while_statement(cond_expr, stmt) );
}

void ast_node_while_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_while_statement::get_typename() const{
	return string();
}

h_ast_node_expression ast_node_while_statement::get_condition_expression() const{
	return cond_expr_;
}

h_ast_node_statement ast_node_while_statement::get_statement() const{
	return stmt_;
}

ast_node_while_statement::ast_node_while_statement( h_ast_node_expression cond_expr, h_ast_node_statement stmt )
: cond_expr_(cond_expr), stmt_(stmt){
}

///////////////////////////////////
//  ast_node_do_while_statement  //
///////////////////////////////////
h_ast_node_do_while_statement ast_node_do_while_statement::create( h_ast_node_statement stmt, h_ast_node_expression cond_expr ){
	return h_ast_node_do_while_statement( new ast_node_do_while_statement(stmt, cond_expr) );
}

void ast_node_do_while_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_do_while_statement::get_typename() const{
	return string();
}

h_ast_node_statement ast_node_do_while_statement::get_statement() const{
	return stmt_;
}

h_ast_node_expression ast_node_do_while_statement::get_condition_expression() const{
	return cond_expr_;
}

ast_node_do_while_statement::ast_node_do_while_statement( h_ast_node_statement stmt, h_ast_node_expression cond_expr )
: stmt_(stmt), cond_expr_(cond_expr){
}

//////////////////////////////
//  ast_node_for_statement  //
//////////////////////////////
h_ast_node_for_statement ast_node_for_statement::create( h_ast_node_statement init_stmt, h_ast_node_expression cond_expr, h_ast_node_expression loop_expr, h_ast_node_statement body ){
	return h_ast_node_for_statement( new ast_node_for_statement( init_stmt, cond_expr, loop_expr, body) );
}

void ast_node_for_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_for_statement::get_typename() const{
	return string();
}

h_ast_node_statement ast_node_for_statement::get_initiaiize_statement() const{
	return init_stmt_;
}

h_ast_node_expression ast_node_for_statement::get_condition_expression() const{
	return cond_expr_;
}

h_ast_node_expression ast_node_for_statement::get_loop_expression() const{
	return loop_expr_;
}

h_ast_node_statement ast_node_for_statement::get_statement() const{
	return body_;
}

ast_node_for_statement::ast_node_for_statement( h_ast_node_statement init_stmt, h_ast_node_expression cond_expr, h_ast_node_expression loop_expr, h_ast_node_statement body )
: init_stmt_(init_stmt), cond_expr_(cond_expr), loop_expr_(loop_expr), body_(body){
}

//////////////////////////////////
//  ast_node_control_statement  //
//////////////////////////////////
h_ast_node_control_statement ast_node_control_statement::create( const std::string& stmt_str, h_ast_node_expression expr ){
	control_statements ctrl_stmt( control_statements::from_name( string("_") + string(stmt_str) ) );
	return h_ast_node_control_statement( new ast_node_control_statement(ctrl_stmt, expr) );
}

h_ast_node_control_statement ast_node_control_statement::create( const control_statements& stmt, h_ast_node_expression expr ){
	return h_ast_node_control_statement( new ast_node_control_statement(stmt, expr) );
}

void ast_node_control_statement::visit( ast_visitor* visitor ){
}

std::string ast_node_control_statement::get_typename() const{
	return string();
}

control_statements ast_node_control_statement::get_control_statement() const{
	return ctrl_stmt_;
}

h_ast_node_expression ast_node_control_statement::get_expression() const{
	return expr_;
}

ast_node_control_statement::ast_node_control_statement( control_statements ctrl_stmt, h_ast_node_expression expr )
: ctrl_stmt_(ctrl_stmt), expr_(expr){
}

/////////////////////
//  ast_node_label //
/////////////////////
h_ast_node_label ast_node_label::create(labels lbl){
	return h_ast_node_label( new ast_node_label(lbl) );
}

h_ast_node_label ast_node_label::create(labels lbl, h_ast_node_identifier ident){
	return h_ast_node_label( new ast_node_label(lbl, ident) );
}

h_ast_node_label ast_node_label::create(labels lbl, h_ast_node_expression expr){
	return h_ast_node_label( new ast_node_label(lbl, expr) );
}

void ast_node_label::visit(ast_visitor *visitor){
}

string ast_node_label::get_typename() const{
	return string();
}

labels ast_node_label::get_label_type() const{
	return lbltype_;
}

h_ast_node_identifier ast_node_label::get_label() const{
	return ident_;
}

h_ast_node_expression ast_node_label::get_expression() const{
	return expr_;
}

ast_node_label::ast_node_label( labels lbl )
: lbltype_(lbl){
}

ast_node_label::ast_node_label(labels lbl, h_ast_node_expression expr)
: lbltype_(lbl), expr_(expr){
}

ast_node_label::ast_node_label(labels lbl, h_ast_node_identifier ident)
: lbltype_(lbl), ident_(ident){
}

//////////////////////////////////
//  ast_node_labeled_statement  //
//////////////////////////////////
h_ast_node_labeled_statement ast_node_labeled_statement::create(h_ast_node_label lbl, h_ast_node_statement stmt){
	return h_ast_node_labeled_statement( new ast_node_labeled_statement( lbl, stmt ) );
}

void ast_node_labeled_statement::visit(ast_visitor *visitor){
	return;
}

string ast_node_labeled_statement::get_typename() const{
	return string();
}

h_ast_node_label ast_node_labeled_statement::get_label() const{
	return lbl_;
}

h_ast_node_statement ast_node_labeled_statement::get_statement() const{
	return stmt_;
}

ast_node_labeled_statement::ast_node_labeled_statement(h_ast_node_label lbl, h_ast_node_statement stmt)
: lbl_( lbl ), stmt_( stmt ){
}