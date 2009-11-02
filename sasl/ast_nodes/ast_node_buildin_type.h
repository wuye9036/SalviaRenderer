#ifndef SASL_AST_NODE_BUILDIN_TYPE_H
#define SASL_AST_NODE_BUILDIN_TYPE_H

#include "ast_node_type.h"
#include "../utility/declare_handle.h"
#include "../enums/buildin_types.h"

DECL_HANDLE( ast_node_buildin_type )
class ast_node_buildin_type: public ast_node_type{
};

DECL_HANDLE( ast_node_scalar_type );
class ast_node_scalar_type: public ast_node_buildin_type{
public:
	static h_ast_node_scalar_type create(const std::string& name);

	// inherited by ast_node
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	virtual size_t hash_value() const;
	virtual bool is_equivalence( h_ast_node_type rhs ) const;

	buildin_types get_buildin_typecode() const;
private:
	ast_node_scalar_type(const buildin_types& typecode);
	buildin_types typecode_;
};

DECL_HANDLE( ast_node_vector_type );
class ast_node_vector_type: public ast_node_buildin_type{
public:
	static h_ast_node_vector_type create(h_ast_node_scalar_type scalar_type, h_ast_node_expression len);

	// inherited by ast_node
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	virtual size_t hash_value() const;
	virtual bool is_equivalence( h_ast_node_type rhs ) const;

	buildin_types get_buildin_typecode() const;
	h_ast_node_scalar_type get_scalar_type();
	h_ast_node_expression length();

private:
	ast_node_vector_type(h_ast_node_scalar_type scalar_type, h_ast_node_expression len);
	h_ast_node_scalar_type scalar_type_;
	h_ast_node_expression length_;
};

DECL_HANDLE( ast_node_matrix_type );
class ast_node_matrix_type: public ast_node_buildin_type{
public:
	static h_ast_node_matrix_type create(h_ast_node_scalar_type scalar_type, h_ast_node_expression row_cnt, h_ast_node_expression col_cnt);

	// inherited by ast_node
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	virtual size_t hash_value() const;
	virtual bool is_equivalence( h_ast_node_type rhs ) const;

	buildin_types get_buildin_typecode() const;
	h_ast_node_scalar_type get_scalar_type();
	h_ast_node_expression column_count();
	h_ast_node_expression row_count();
private:
	ast_node_matrix_type(h_ast_node_scalar_type scalar_type, h_ast_node_expression rowcnt, h_ast_node_expression colcnt);
	h_ast_node_scalar_type scalar_type_;
	h_ast_node_expression rowcnt_, colcnt_;
};

#endif
