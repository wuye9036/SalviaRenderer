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
struct cg_value;
struct cg_function;
struct instruction_t;
struct ssa_attribute;

class ssa_context
{
public:
	cg_function*		create_function();
	cg_value*		create_value( block_t* parent, sasl::syntax_tree::node* attached );
	variable_t*		create_variable( cg_function* parent, sasl::syntax_tree::node* decl );
	block_t*		create_block( cg_function* parent );
	instruction_t*	emit( block_t* parent, int id );
	instruction_t*	emit( block_t* parent, instruction_t* position, int id );

	ssa_attribute&	attr( sasl::syntax_tree::node* n );

	cg_value*		load( sasl::syntax_tree::node* n );
	cg_value*		load( variable_t* var );
	void			store( variable_t* var, cg_value* val );
private:
	instruction_t*	emit_null();
	void			init_value( cg_value* v, block_t* parent, int vid, sasl::syntax_tree::node* attached );

	typedef boost::unordered_map< sasl::syntax_tree::node*, ssa_attribute* > ssa_attrs_t;
	typedef ssa_attrs_t::iterator ssa_attr_iter_t;
	ssa_attrs_t ssa_attrs;

	std::vector<variable_t*>	vars;
	std::vector<cg_function*>	fns;
	std::vector<cg_value*>		values;
	std::vector<block_t*>		blocks;
	std::vector<instruction_t*>	instructions;
};

END_NS_SASL_SEMANTIC();

#endif