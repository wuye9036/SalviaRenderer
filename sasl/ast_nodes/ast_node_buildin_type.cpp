#include "ast_node_buildin_type.h"

#include <boost/format.hpp>

using namespace boost;
using namespace std;

////////////////////////////
//  ast_node_scalar_type  //
////////////////////////////
h_ast_node_scalar_type ast_node_scalar_type::create(const string& name){
	//根据类型别名创建
	if( name == "int"){
		return h_ast_node_scalar_type(new ast_node_scalar_type(buildin_types::sasl_int32));
	}
	
	//根据类型名称创建
	return h_ast_node_scalar_type( new ast_node_scalar_type(buildin_types::from_name("sasl_" + name)) );
	return h_ast_node_scalar_type();
}

ast_node_scalar_type::ast_node_scalar_type(const buildin_types& typecode):typecode_(typecode){
}

buildin_types ast_node_scalar_type::get_buildin_typecode() const{
	return typecode_;
}

string ast_node_scalar_type::get_typename() const{
	return string();
}

void ast_node_scalar_type::release(){
	delete this;
}

void ast_node_scalar_type::visit(ast_visitor* visitor){
	visitor->enter_node( self_ );
	visitor->leave_node( self_ );
}

size_t ast_node_scalar_type::hash_value() const{
	return typecode_.to_value();
}

bool ast_node_scalar_type::is_equivalence( h_ast_node_type rhs ) const{
	return rhs->get_buildin_typecode() == get_buildin_typecode();
}
////////////////////////////
//  ast_node_vector_type  //
////////////////////////////
h_ast_node_vector_type ast_node_vector_type::create(h_ast_node_scalar_type scalar_type, h_ast_node_expression len){
	return h_ast_node_vector_type(new ast_node_vector_type(scalar_type, len));
}

string ast_node_vector_type::get_typename() const{
	return string();
}

void ast_node_vector_type::visit(ast_visitor* visitor){
	visitor->enter_node( self_ );
	{
		scalar_type_->visit( visitor );
		length_->visit( visitor );
	}
	visitor->leave_node( self_ );

}

void ast_node_vector_type::release(){
	delete this;
}

h_ast_node_scalar_type ast_node_vector_type::get_scalar_type(){
	return scalar_type_;
}

h_ast_node_expression ast_node_vector_type::length(){
	return length_;
}

ast_node_vector_type::ast_node_vector_type(h_ast_node_scalar_type scalar_type, h_ast_node_expression len)
	:scalar_type_(scalar_type), length_(len){
}

buildin_types ast_node_vector_type::get_buildin_typecode() const{
	return buildin_types::sasl_vector;
}

size_t ast_node_vector_type::hash_value() const{
	return 
		(( (size_t)(buildin_types::sasl_vector.to_value()) + scalar_type_->hash_value() ) << 4) + length_->constant_value_T<int>()
		;
}

bool ast_node_vector_type::is_equivalence( h_ast_node_type rhs ) const {
	if( get_buildin_typecode() != rhs->get_buildin_typecode() ){
		return false;
	}

	h_ast_node_vector_type rhs_vector_type = boost::static_pointer_cast< ast_node_vector_type >( rhs );
	return 
		scalar_type_->is_equivalence( rhs_vector_type->scalar_type_ )
		&& length_->constant_value_T<int>() == rhs_vector_type->length_->constant_value_T<int>();
}

h_ast_node_matrix_type ast_node_matrix_type::create(h_ast_node_scalar_type scalar_type, h_ast_node_expression row_cnt, h_ast_node_expression col_cnt){
	return h_ast_node_matrix_type( new ast_node_matrix_type(scalar_type, row_cnt, col_cnt) );
}

ast_node_matrix_type::ast_node_matrix_type(h_ast_node_scalar_type scalar_type, h_ast_node_expression rowcnt, h_ast_node_expression colcnt)
	:scalar_type_(scalar_type), rowcnt_(rowcnt), colcnt_(colcnt){
}
string ast_node_matrix_type::get_typename() const{
	return string();
}

void ast_node_matrix_type::visit(ast_visitor* visitor){
	visitor->enter_node( self_ );
	{
		scalar_type_->visit( visitor );
		rowcnt_->visit( visitor );
		colcnt_->visit( visitor );
	}
	visitor->leave_node( self_ );
}

void ast_node_matrix_type::release(){
}

h_ast_node_scalar_type ast_node_matrix_type::get_scalar_type(){
	return scalar_type_;
}

h_ast_node_expression ast_node_matrix_type::column_count(){
	return colcnt_;
}

h_ast_node_expression ast_node_matrix_type::row_count(){
	return rowcnt_;
}

buildin_types ast_node_matrix_type::get_buildin_typecode() const{
	return buildin_types::sasl_matrix;
}

size_t ast_node_matrix_type::hash_value() const {
	return
		(( buildin_types::sasl_matrix.to_value() + scalar_type_->hash_value() ) << 8 )
		+ (rowcnt_->constant_value_T<int>() << 4 )
		+ colcnt_->constant_value_T<int>()
		;
}