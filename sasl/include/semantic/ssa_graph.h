#ifndef SASL_SEMANTIC_SSA_GRAPH_H
#define SASL_SEMANTIC_SSA_GRAPH_H

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

struct block_t;
struct instruction_t;
struct function_t;
struct variable_t;
struct value_t;

class ssa_graph
{
public:
	std::vector<function_t*>	functions() const;
	std::vector<variable_t*>	globals() const;
	function_t*					ssa_fn( sasl::syntax_tree::node* fn ) const;
	ssa_context*				context();
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
	block_t*		idom_block( block_t* b );
	block_t*		pidom_block( block_t* b );

	bool			dominance( instruction_t* i0,  instruction_t* i1 ); // i0 dom i1
	bool			idominance( instruction_t* i0,  instruction_t* i1 ); // i0 dom i1
	bool			post_dominance( instruction_t* i0,  instruction_t* i1 ); //i0 pdom i1
	bool			post_idominance( instruction_t* i0,  instruction_t* i1 ); //i0 pidom i1;

	bool			dominance( block_t* b0, block_t* b1 ); // b0 dom b1
	bool			idominance( block_t* b0, block_t* b1 );	//b0 idom b1;
	bool			post_dominance( block_t* b0, block_t* b1 ); //b0 pdom b1
	bool			post_idominance( block_t* b0, block_t* b1 ); //b0 pidom b1;

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

struct dom_tree_node
{
	dom_tree_node*	idom;
	dom_tree_node*	pdom;
	
	size_t			idom_post_order;
	size_t			pdom_post_order;

	block_t*		block;
};

struct execution_modes
{
	enum execution_mode{
		em_unknown,
		em_single,
		em_multiple,
	};
public:
	static boost::shared_ptr<dom_tree> compute_execution_modes( module_si*, ssa_graph* );
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