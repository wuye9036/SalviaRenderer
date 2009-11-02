#ifndef SASL_AST_NODE_TYPE_H
#define SASL_AST_NODE_TYPE_H

#include "ast_node.h"

DECL_HANDLE( ast_node_declaration );
DECL_HANDLE( ast_node_statement );

DECL_HANDLE( ast_node_type )
class ast_node_type: public ast_node{
public:
	std::vector<type_qualifiers>& get_type_qualifiers();
	const std::vector<type_qualifiers>& get_type_qualifiers() const;
	
	bool has_qualifier(const type_qualifiers& qual) const;
	
	virtual size_t hash_value() const = 0;

	// if rhs has a additional type qualifier and ignore_add_qualifier is true, it considers that two types are equivalent.
	// if this has a additional type qualifier and ignore_remove_qualifier is true, it considers that two types are equivalent.
	// for eg:
	//     const int is equivalence int while ignore_remove_qualifier is true.
	// this feature is useful in this case that judging equivalence of two types while assignment.
	// in default, type "this" is the origin type while "rhs" is the target one, 
	// adding a qualifier from origin to targe will be allowed when type converted.
	bool is_equivalence( h_ast_node_type rhs, bool ignore_add_qualifier, bool ignore_remove_qualifer ) const;

	// judge types equivalence ignoring qualifier
	virtual bool is_equivalence( h_ast_node_type rhs ) const = 0;
	virtual buildin_types get_buildin_typecode() const;

protected:	
	ast_node_type( const std::vector<type_qualifiers>& quals, const ast_node_types& node_type = ast_node_types::type );
	ast_node_type( const ast_node_types& node_type = ast_node_types::type );

	std::vector<type_qualifiers> quals_;
};

DECL_HANDLE( ast_node_function )
class ast_node_function: public ast_node_type{
public:
	static h_ast_node_function create(
		h_ast_node_type ret_type,
		const std::vector<h_ast_node_variable_declaration>& params,
		h_ast_node_statement body
		);

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	size_t hash_value() const;
	bool is_equivalence( h_ast_node_type rhs ) const;
	
	h_ast_node_type get_return_type() const;
	const std::vector< h_ast_node_variable_declaration >& get_parameters() const;
	h_ast_node_statement get_function_body() const;

private:
	ast_node_function(
		h_ast_node_type ret_type,
		const std::vector<h_ast_node_variable_declaration>& params,
		h_ast_node_statement body
		);

	h_ast_node_type ret_type_;
	std::vector<h_ast_node_variable_declaration> params_;
	h_ast_node_statement body_;
};

DECL_HANDLE( ast_node_array )
class ast_node_array: public ast_node_type{
public:
	static h_ast_node_array create( 
		h_ast_node_type elem_type,
		const std::vector<h_ast_node_expression>& size_exprs );

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	size_t hash_value() const;
	bool is_equivalence( h_ast_node_type rhs ) const;

	std::vector<h_ast_node_expression>& get_size_expressions();
	const std::vector<h_ast_node_expression>& get_size_expressions() const;

	h_ast_node_type get_element_type() const;
private:
	ast_node_array( 
		h_ast_node_type elem_type,
		const std::vector<h_ast_node_expression>& size_exprs 
		);
	h_ast_node_type elem_type_;
	std::vector<h_ast_node_expression> size_exprs_;
};

DECL_HANDLE( ast_node_struct );
class ast_node_struct: public ast_node_type{
public:
	static h_ast_node_struct create( h_ast_node_identifier ident, const std::vector<h_ast_node_declaration>& members );

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	size_t hash_value() const;
	bool is_equivalence( h_ast_node_type rhs ) const;

	const std::vector<h_ast_node_declaration>& get_members() const;
	h_ast_node_identifier get_identifier() const;

private:
	ast_node_struct(h_ast_node_identifier ident, const std::vector<h_ast_node_declaration>& members);
	std::vector<h_ast_node_declaration> members_;
	h_ast_node_identifier ident_;
};
DECL_HANDLE( ast_node_identifier_type )
class ast_node_identifier_type : public ast_node_type{
public:
	static h_ast_node_identifier_type create( h_ast_node_identifier ident );

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	size_t hash_value() const;
	bool is_equivalence( h_ast_node_type rhs ) const;

	h_ast_node_identifier get_identifier() const;
private:
	ast_node_identifier_type( h_ast_node_identifier ident );
	h_ast_node_identifier ident_;
};
#endif