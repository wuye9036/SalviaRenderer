#ifndef SASL_SYNTAX_TREE_DECLARATION_H
#define SASL_SYNTAX_TREE_DECLARATION_H

#include <sasl/syntax_tree/syntax_tree_fwd.h>
#include <sasl/syntax_tree/node.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/node_ids.h>
#include <sasl/enums/type_qualifiers.h>

#include <eflib/utility/enable_if.h>
//#include <eflib/utility/util.h>

#include <memory>
#include <vector>

namespace sasl
{
	namespace common
	{
		struct token_t;
	}
}

namespace sasl::syntax_tree {

class syntax_tree_visitor;

EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
EFLIB_DECLARE_STRUCT_SHARED_PTR(compound_statement);
EFLIB_DECLARE_STRUCT_SHARED_PTR(statement);
EFLIB_DECLARE_STRUCT_SHARED_PTR(expression);
EFLIB_DECLARE_STRUCT_SHARED_PTR(function_type);

using sasl::common::token_t;

struct initializer: public node{
protected:
	initializer(node_ids type_id, std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	initializer& operator = ( const initializer& );
	initializer( const initializer& );
};

struct expression_initializer: public initializer{

	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::shared_ptr< expression > init_expr;
private:
	expression_initializer( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	expression_initializer& operator = ( const expression_initializer& );
	expression_initializer( const expression_initializer& );
};

struct member_initializer: public initializer{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::vector< std::shared_ptr<initializer> > sub_inits;
private:
	member_initializer( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	member_initializer& operator = ( const member_initializer& );
	member_initializer( const member_initializer& );
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(declaration);
struct declaration: public node{
protected:
	declaration(node_ids type_id, std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	declaration& operator = ( const declaration& );
	declaration( const declaration& );
};

struct declarator: public node{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<token_t>		name;
	std::shared_ptr<initializer>	init;
	std::shared_ptr<token_t>		semantic;
	std::shared_ptr<token_t>		semantic_index;

protected:
	declarator( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	declarator& operator = ( const declarator& );
	declarator( const declarator& );
};

struct variable_declaration : public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<tynode>						type_info;
	std::vector< std::shared_ptr<declarator> >	declarators;

protected:
	variable_declaration(std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end);
	variable_declaration& operator = ( const variable_declaration& );
	variable_declaration( const variable_declaration& );
};

struct type_definition: public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::shared_ptr< tynode > type_info;
	std::shared_ptr<token_t> name;

protected:
	type_definition( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	type_definition& operator = ( const type_definition& );
	type_definition( const type_definition& );
};


struct tynode: public declaration{
	builtin_types tycode;
	type_qualifiers qual;

	bool is_builtin() const;
	bool is_struct() const;
	bool is_array() const;
	bool is_function() const;
	bool is_alias() const;

	bool is_uniform() const;
protected:
	tynode(node_ids type_id, std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end);
};

struct alias_type: public tynode{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::shared_ptr<token_t> alias;
protected:
	alias_type( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	alias_type& operator = ( const alias_type& );
	alias_type( const alias_type& );
};

struct builtin_type: public tynode{
public:
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	bool is_builtin() const;

protected:
	builtin_type( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	builtin_type& operator = ( const builtin_type& ) = delete;
	builtin_type( const builtin_type& ) = delete;
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(array_type);
struct array_type: public tynode{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::vector< std::shared_ptr<expression> > array_lens;
	std::shared_ptr< tynode > elem_type;
protected:
	array_type( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	array_type& operator = ( const array_type& );
	array_type( const array_type& );
};

struct struct_type: public tynode{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::shared_ptr< token_t > name;
	bool has_body;
	std::vector< std::shared_ptr<declaration> > decls;

protected:
	struct_type( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	struct_type& operator = ( const struct_type& );
	struct_type( const struct_type& );
};

struct parameter_full: public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<tynode>		param_type;
	std::shared_ptr<token_t>		name;
	std::shared_ptr<initializer>	init;
	std::shared_ptr<token_t>		semantic;
	std::shared_ptr<token_t>		semantic_index;

protected:
	parameter_full(
		std::shared_ptr<token_t> const& tok_beg,
		std::shared_ptr<token_t> const& tok_end
		);
	parameter_full& operator = (parameter_full const &);
	parameter_full(parameter_full const &);
};

struct function_full_def: public tynode{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr< token_t >						name;
	std::shared_ptr< tynode >							retval_type;
	std::vector< std::shared_ptr<parameter_full> >	params;
	std::shared_ptr<token_t>							semantic;
	std::shared_ptr<token_t>							semantic_index;
	std::shared_ptr<compound_statement>				body;

	bool declaration_only();

	~function_full_def()
	{
		;
	}
protected:
	function_full_def( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
	function_full_def& operator = ( const function_full_def& );
	function_full_def( const function_full_def& );
};

struct parameter: public declaration
{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<token_t>	   name;
	std::shared_ptr<initializer> init;
	std::shared_ptr<token_t>	   semantic;
	std::shared_ptr<token_t>	   semantic_index;

protected:
	parameter(
		std::shared_ptr<token_t> const& tok_beg,
		std::shared_ptr<token_t> const& tok_end
		);
	parameter& operator = ( parameter const & );
	parameter( parameter const & );
};

struct function_def: public declaration
{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::shared_ptr<function_type>			type;

	std::shared_ptr<token_t>					name;
	std::vector< std::shared_ptr<parameter> > params;
	std::shared_ptr<token_t>					semantic;
	std::shared_ptr<token_t>					semantic_index;

	std::shared_ptr<compound_statement>		body;

	bool declaration_only();

	~function_def()
	{
		;
	}
protected:
	function_def(
		std::shared_ptr<token_t> const& tok_beg,
		std::shared_ptr<token_t> const& tok_end
		);
	function_def& operator = ( const function_def& );
	function_def( const function_def& );
};

struct function_type: public tynode
{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::vector<tynode_ptr>	param_types;
	tynode_ptr				result_type;

protected:
	function_type(
		std::shared_ptr<token_t> const& tok_beg,
		std::shared_ptr<token_t> const& tok_end
		);
	function_type& operator = ( const function_def& );
	function_type( const function_def& );
};

struct null_declaration: public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
protected:
	null_declaration( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end );
};
}

#endif
