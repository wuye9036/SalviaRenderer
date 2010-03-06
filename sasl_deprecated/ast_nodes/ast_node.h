#ifndef SASL_AST_NODE_H
#define SASL_AST_NODE_H

#include "../visitors/ast_visitor.h"

#include "../enums/ast_node_types.h"
#include "../enums/type_qualifiers.h"
#include "../enums/buildin_types.h"

#include "../utility/declare_handle.h"
#include <boost/any.hpp>
#include <vector>

class ast_visitor;

class ast_node_type_qualifier;
DECL_HANDLE( ast_node_type_qualifier );
class ast_node_variable_declaration;
DECL_HANDLE( ast_node_variable_declaration );
class ast_node_expression;
DECL_HANDLE( ast_node_expression );

DECL_HANDLE( ast_node );
class ast_node{
public:
	virtual std::string get_typename() const = 0;
	virtual ast_node_types get_node_type() const{
		return type_;
	}
	virtual void visit(ast_visitor* visitor) = 0;
	virtual void release(){}

protected:
	ast_node& operator = (const ast_node& );
	ast_node():type_(ast_node_types::unknown){}
	ast_node(const ast_node& rhs):type_(rhs.type_) {}
	ast_node(const ast_node_types& node_type):type_(node_type){}
	
	WEAK_HANDLE_OF( ast_node ) self_;
	ast_node_types type_;
	virtual ~ast_node(){}
};

DECL_HANDLE( ast_node_identifier );
class ast_node_identifier: public ast_node{
public:
	static h_ast_node_identifier create(const std::string& ident);

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	//others
	const std::string& get_ident();
private:
	ast_node_identifier( const std::string& ident );
	std::string ident_;
};

DECL_HANDLE( ast_node_expression );
class ast_node_expression: public ast_node{
public:
	virtual bool is_const_expression() const{ return false; }
	virtual boost::any constant_value() = 0;
	template< typename T > T constant_value_T(){
		return boost::any_cast<T>(constant_value());
	}
protected:
	ast_node_expression(const ast_node_types& node_type)
		: ast_node( node_type ){
	}
	ast_node_expression():ast_node( ast_node_types::expression ){
	}
};

DECL_HANDLE( ast_node_list );
class ast_node_list: public ast_node{
public:
	static h_ast_node_list create();

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);

	virtual void add_node( h_ast_node node );
private:
	ast_node_list();
	std::vector<h_ast_node> node_list_;
};

#endif