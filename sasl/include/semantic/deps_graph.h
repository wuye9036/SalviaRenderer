#ifndef SASL_SEMANTIC_DEPS_GRAPH_H
#define SASL_SEMANTIC_DEPS_GRAPH_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
		struct function_type;
	}
}

BEGIN_NS_SASL_SEMANTIC();

class module_si;
class ssa_context;
struct instruction_t;
struct block_t;
struct variable_t;
struct value_t;

struct instruction_t
{
	instruction_t();

	enum IDs
	{
		none,
		load,
		save,
		phi,
		eval
	} id;

	// variable or parameters
	variable_t*				var;
	std::vector<value_t*>	params;

	block_t*		parent;
	instruction_t*	next;
	instruction_t*	prev;
};

struct block_t
{
	block_t();

	void push_back( instruction_t* ins );
	void insert( instruction_t* ins, instruction_t* pos );
	bool empty();

	instruction_t* beg;
	instruction_t* end;
	
	std::vector<block_t*>		preds;
	std::vector< std::pair<value_t*,block_t*> >		succs;
};

struct variable_t
{
	sasl::syntax_tree::node*	decl;
	std::vector<size_t>			members;
	value_t*					value;
};

struct value_t
{
	block_t* parent;

	// Expr node maybe: declarator or expression
	sasl::syntax_tree::node*	expr_node;

	instruction_t*				ins;
};

struct function_t
{
	sasl::syntax_tree::function_type* fn;

	value_t*	retval;	
	block_t*	entry;
	block_t*	exit;
};

class ssa_graph
{
public:
	function_t*		ssa_fn( sasl::syntax_tree::node* fn ) const;
	ssa_context*	context();
private:
	boost::shared_ptr<ssa_context> ctxt;
};

struct ssa_attribute
{
	value_t*	val;
	variable_t*	var;
	function_t*	fn;
};

struct dom_tree_node;
class dom_tree
{
public:
	static boost::shared_ptr<dom_tree> construct_dom_tree( module_si*, ssa_graph* );
	
	dom_tree_node*	dom_node( block_t* b );
	block_t*		dom_block( block_t* b );
private:
	boost::unordered_map<function_t*, dom_tree_node*>	dom_roots;
	boost::unordered_map<block_t*, dom_tree_node*>		dom_nodes;
};

struct block_vmap
{
	block_t* block;
	typedef std::pair<instruction_t*, value_t*> pos_value_pair_t;
	boost::unordered_map< variable_t*, std::vector<pos_value_pair_t> > block_variables;
};

class function_vmap
{
public:
	void		construct_vmap( function_t* fn );
	void		store( instruction_t* position, variable_t* var, value_t* v );
	value_t*	load ( instruction_t* position, variable_t* var );

private:
	boost::unordered_map<block_t*, block_vmap> block_variables;
};

void simplify_phi_node( ssa_graph* g );

struct dom_tree_node
{
	dom_tree_node*	idom;
	dom_tree_node*	pdom;
	
	size_t			idom_post_order;
	size_t			pdom_post_order;

	block_t*		block;
};

class dom_frontiers
{

};

// address_ident_t
//  r-value expression
//  variable / l-value expression
//  member of variable.

class address_ident_t
{
public:
	bool operator == ( address_ident_t const& rhs ) const;

	enum data_types
	{
		unknown,
		constant,
		expr,
		var,
		mem
	};

	explicit address_ident_t( sasl::syntax_tree::node* nd );
	template <typename IteratorT>
	address_ident_t( sasl::syntax_tree::node* nd, IteratorT begin, IteratorT end )
		: agg(nd), mem_indexes(begin, end)
	{
	}
	address_ident_t member_of( size_t index ) const;
	address_ident_t parent_of() const;

	size_t hash_value() const;
private:
	sasl::syntax_tree::node*	agg;
	std::vector<size_t>			mem_indexes;
};

size_t hash_value( address_ident_t const& v );

class deps_graph{
public:
	enum dep_kinds
	{
		unknown,
		affects,
		depends,
		aggr_of,
		part_of,
	};

	static boost::shared_ptr<deps_graph> create();

	void add( address_ident_t const&, address_ident_t const&, dep_kinds dep_kind );

	std::vector<address_ident_t> inputs_of( address_ident_t const& src ) const;
	std::vector<address_ident_t> outputs_of( address_ident_t const& src ) const;

private:
	deps_graph() {}
	deps_graph( deps_graph const& );
	deps_graph& operator = ( deps_graph const& );

	typedef boost::unordered_multimap< std::pair<address_ident_t, address_ident_t>, dep_kinds > v2e_t;
	v2e_t v2e;
	typedef boost::unordered_multimap< std::pair<address_ident_t, dep_kinds>, address_ident_t > v2v_t;
	v2v_t v2v;
};

END_NS_SASL_SEMANTIC();

#endif