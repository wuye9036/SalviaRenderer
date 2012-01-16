#ifndef SASL_SEMANTIC_DEPS_GRAPH_H
#define SASL_SEMANTIC_DEPS_GRAPH_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

struct expr_t;
struct block_t;
struct variable_t;
struct value_t;

struct expr_t
{
	variable_t*	var;
	value_t*	val;
	std::vector<value_t> params;
};

struct block_t
{
	std::vector<block_t*> preds;
	std::vector<block_t*> succs;
	std::vector<expr_t*> exprs;
};

struct variable_t
{
	sasl::syntax_tree::node*	decl;
	std::vector<size_t>			members;
	value_t*					value;
};

struct value_t
{
	std::vector< std::pair<block_t*, sasl::syntax_tree::node*> > phi_exprs;
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