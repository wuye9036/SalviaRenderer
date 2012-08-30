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

class module_semantic;
class ssa_context;

struct block_t;
struct instruction_t;
struct cg_function;
struct variable_t;
struct cg_value;

class ssa_graph
{
public:
	std::vector<cg_function*>	functions() const;
	std::vector<variable_t*>	global_vars() const;
	cg_function*					ssa_fn( sasl::syntax_tree::node* fn ) const;
	ssa_context*				context();
private:
	boost::shared_ptr<ssa_context> ctxt;
};

struct ssa_attribute
{
	cg_value*	val;
	variable_t*	var;
	cg_function*	fn;
};

struct dom_tree_node;
class dom_tree
{
public:
	static boost::shared_ptr<dom_tree> construct_dom_tree( module_semantic*, ssa_graph* );
	
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
	boost::unordered_map<cg_function*, dom_tree_node*>	dom_roots;
	boost::unordered_map<block_t*, dom_tree_node*>		dom_nodes;
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
	static boost::shared_ptr<dom_tree> compute_execution_modes( module_semantic*, ssa_graph* );
};

END_NS_SASL_SEMANTIC();

#endif