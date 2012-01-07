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

// data_t
//  expression
//  variable
//  member of variable.

class data_t
{
public:
	enum data_types
	{
		unknown,
		constant,
		expr,
		var,
		mem
	};

	data_t( sasl::syntax_tree::node* nd );

	data_t member_of( sasl::syntax_tree::node* mem ) const;
	data_t parent_of() const;

private:
	std::vector<sasl::syntax_tree::node*> member_list;
};

bool operator == ( data_t const& lhs, data_t const& rhs ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return false;
}

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

	void add( data_t, data_t, dep_kinds dep_kind );

	void inputs_of( sasl::syntax_tree::node* src );
	void outputs_of( sasl::syntax_tree::node* src );

private:
	boost::unordered_multimap< std::pair<data_t, data_t>, dep_kinds > v2e;
	boost::unordered_multimap< std::pair<data_t, data_t>, sasl::syntax_tree::node* > v2v;
};

END_NS_SASL_SEMANTIC();

#endif