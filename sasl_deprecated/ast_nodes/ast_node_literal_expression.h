#ifndef SASL_AST_NODE_LITERAL_H
#define SASL_AST_NODE_LITERAL_H

#include "ast_node_expression.h"
#include "ast_node.h"

#include "../enums/literal_types.h"

#include <string>
#include <boost/lexical_cast.hpp>

DECL_HANDLE( ast_node_literal_expression );
class ast_node_literal_expression: public ast_node_expression{
public:
	static h_ast_node_literal_expression create( const std::string& literal, const std::string& type_suffix, const literal_types& lit_type );
	std::string get_literal();
	std::string get_type_suffix();
	literal_types get_literal_type();

	virtual bool is_const_expression() const{
		return true;
	}
	virtual boost::any constant_value();
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

private:
	ast_node_literal_expression( const std::string& literal, const std::string& type_suffix, const literal_types& lit_type );
	std::string literal_;
	std::string type_suffix_;
	literal_types lit_type_;
};

template<typename ValueT>
class ast_node_value_expression: public ast_node_expression{
public:
	static h_ast_node_expression create(const ValueT& val){
		return h_ast_node_expression( new ast_node_value_expression(val) );
	}

	ValueT get_value(){
		return val_;
	}
	
	virtual bool is_const_expression() const{
		return true;
	}
	virtual boost::any constant_value(){
		return boost::any(val_);
	}
	virtual std::string get_typename() const{
		return std::string();
	}

	virtual void visit(ast_visitor* visitor){
		//do nothing
	}

	virtual void release(){
		delete this;
	}

private:
	ast_node_value_expression(const ValueT& val): val_(val){
	}

	ValueT val_;
};

#endif