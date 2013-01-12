#ifndef SASL_SYNTAX_TREE_DECLARATION_H
#define SASL_SYNTAX_TREE_DECLARATION_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/node_ids.h>
#include <sasl/enums/type_qualifiers.h>

#include <eflib/include/utility/enable_if.h>
//#include <eflib/include/utility/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl
{
	namespace common
	{
		struct token_t;
	}
}

BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_visitor;

EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
EFLIB_DECLARE_STRUCT_SHARED_PTR(compound_statement);
EFLIB_DECLARE_STRUCT_SHARED_PTR(statement);
EFLIB_DECLARE_STRUCT_SHARED_PTR(expression);
EFLIB_DECLARE_STRUCT_SHARED_PTR(function_type);

using sasl::common::token_t;

struct initializer: public node{
protected:
	initializer(node_ids type_id, boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	initializer& operator = ( const initializer& );
	initializer( const initializer& );
};

struct expression_initializer: public initializer{

	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	boost::shared_ptr< expression > init_expr;
private:
	expression_initializer( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	expression_initializer& operator = ( const expression_initializer& );
	expression_initializer( const expression_initializer& );
};

struct member_initializer: public initializer{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::vector< boost::shared_ptr<initializer> > sub_inits;
private:
	member_initializer( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	member_initializer& operator = ( const member_initializer& );
	member_initializer( const member_initializer& );
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(declaration);
struct declaration: public node{
protected:
	declaration(node_ids type_id, boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	declaration& operator = ( const declaration& );
	declaration( const declaration& );
};

struct declarator: public node{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	boost::shared_ptr<token_t>		name;
	boost::shared_ptr<initializer>	init;
	boost::shared_ptr<token_t>		semantic;
	boost::shared_ptr<token_t>		semantic_index;

protected:
	declarator( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	declarator& operator = ( const declarator& );
	declarator( const declarator& );
};

struct variable_declaration : public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	boost::shared_ptr<tynode>						type_info;
	std::vector< boost::shared_ptr<declarator> >	declarators;

protected:
	variable_declaration(boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end);
	variable_declaration& operator = ( const variable_declaration& );
	variable_declaration( const variable_declaration& );
};

struct type_definition: public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	boost::shared_ptr< tynode > type_info;
	boost::shared_ptr<token_t> name;

protected:
	type_definition( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
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
	tynode(node_ids type_id, boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end);
};

struct alias_type: public tynode{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	boost::shared_ptr<token_t> alias;
protected:
	alias_type( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	alias_type& operator = ( const alias_type& );
	alias_type( const alias_type& );
};

struct builtin_type: public tynode{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	bool is_builtin() const;

protected:
	builtin_type( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	builtin_type& operator = ( const builtin_type& );
	builtin_type( const builtin_type& );
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(array_type);
struct array_type: public tynode{
	SASL_SYNTAX_NODE_CREATORS();

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	std::vector< boost::shared_ptr<expression> > array_lens;
	boost::shared_ptr< tynode > elem_type;
protected:
	array_type( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	array_type& operator = ( const array_type& );
	array_type( const array_type& );
};

struct struct_type: public tynode{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	boost::shared_ptr< token_t > name;
	bool has_body;
	std::vector< boost::shared_ptr<declaration> > decls;

protected:
	struct_type( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	struct_type& operator = ( const struct_type& );
	struct_type( const struct_type& );
};

struct parameter_full: public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	boost::shared_ptr<tynode>		param_type;
	boost::shared_ptr<token_t>		name;
	boost::shared_ptr<initializer>	init;
	boost::shared_ptr<token_t>		semantic;
	boost::shared_ptr<token_t>		semantic_index;

protected:
	parameter_full(
		boost::shared_ptr<token_t> const& tok_beg,
		boost::shared_ptr<token_t> const& tok_end
		);
	parameter_full& operator = (parameter_full const &);
	parameter_full(parameter_full const &);
};

struct function_full_def: public tynode{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	boost::shared_ptr< token_t >						name;
	boost::shared_ptr< tynode >							retval_type;
	std::vector< boost::shared_ptr<parameter_full> >	params;
	boost::shared_ptr<token_t>							semantic;
	boost::shared_ptr<token_t>							semantic_index;
	boost::shared_ptr<compound_statement>				body;

	bool declaration_only();

	~function_full_def()
	{
		;
	}
protected:
	function_full_def( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
	function_full_def& operator = ( const function_full_def& );
	function_full_def( const function_full_def& );
};

struct parameter: public declaration
{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	boost::shared_ptr<token_t>	   name;
	boost::shared_ptr<initializer> init;
	boost::shared_ptr<token_t>	   semantic;
	boost::shared_ptr<token_t>	   semantic_index;

protected:
	parameter(
		boost::shared_ptr<token_t> const& tok_beg,
		boost::shared_ptr<token_t> const& tok_end
		);
	parameter& operator = ( parameter const & );
	parameter( parameter const & );
};

struct function_def: public declaration
{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();

	boost::shared_ptr<function_type>			type;

	boost::shared_ptr<token_t>					name;
	std::vector< boost::shared_ptr<parameter> > params;
	boost::shared_ptr<token_t>					semantic;
	boost::shared_ptr<token_t>					semantic_index;

	boost::shared_ptr<compound_statement>		body;

	bool declaration_only();

	~function_def()
	{
		;
	}
protected:
	function_def(
		boost::shared_ptr<token_t> const& tok_beg,
		boost::shared_ptr<token_t> const& tok_end
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
		boost::shared_ptr<token_t> const& tok_beg,
		boost::shared_ptr<token_t> const& tok_end
		);
	function_type& operator = ( const function_def& );
	function_type( const function_def& );
};

struct null_declaration: public declaration{
	SASL_SYNTAX_NODE_CREATORS();
	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
protected:
	null_declaration( boost::shared_ptr<token_t> const& tok_beg, boost::shared_ptr<token_t> const& tok_end );
};
END_NS_SASL_SYNTAX_TREE();

#endif
