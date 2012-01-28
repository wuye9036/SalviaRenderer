#ifndef SASL_SEMANTIC_SSA_CONTEXT_H
#define SASL_SEMANTIC_SSA_CONTEXT_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl {
	namespace syntax_tree {
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

struct expr_t;
struct block_t;
struct variable_t;
struct value_t;
struct function_t;
struct instruction_t;
struct ssa_attribute;

class ssa_context
{
public:
	function_t*		create_function();
	value_t*		create_value();
	variable_t*		create_variable( sasl::syntax_tree::node* decl );
	block_t*		create_block();
	instruction_t*	create_instruction();
	ssa_attribute&	attr( sasl::syntax_tree::node* n );

	value_t*		load( sasl::syntax_tree::node* n );
	value_t*		load( variable_t* var );
	void			store( variable_t* var, value_t* val );
private:
	typedef boost::unordered_map< sasl::syntax_tree::node*, ssa_attribute* > ssa_attrs_t;
	typedef ssa_attrs_t::iterator ssa_attr_iter_t;
	ssa_attrs_t ssa_attrs;

	std::vector<variable_t*>	vars;
	std::vector<function_t*>	fns;
	std::vector<value_t*>		values;
	std::vector<block_t*>		blocks;
};

END_NS_SASL_SEMANTIC();

#endif