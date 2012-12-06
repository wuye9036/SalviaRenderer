#ifndef SASL_SEMANTIC_SSA_NODES_H
#define SASL_SEMANTIC_SSA_NODES_H

#include <sasl/include/semantic/semantic_forward.h>

#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
		struct function_type;
	}
}

BEGIN_NS_SASL_SEMANTIC();

struct block_t;
struct cg_function;
struct multi_value;
struct variable_t;

struct multi_value
{
	friend class ssa_context;
	enum ID{
		value,
		variable,
		instruction,
		block
	};

	block_t*					parent;
	ssa_context*				context;
	sasl::syntax_tree::node*	attached;
	ID							vid;
protected:
	multi_value(){}
	~multi_value(){}
};

struct instruction_t: public multi_value
{
	friend class ssa_context;
	enum IDs
	{
		none,
		load,
		save,
		phi,
		eval
	} id;

	std::vector<multi_value*>	params;

	instruction_t*	next;
	instruction_t*	prev;
private:
	instruction_t(){}
	~instruction_t(){}
};

struct variable_t: public multi_value
{
	friend class ssa_context;
	std::vector<size_t> members;
	cg_function*			fn;
private:
	variable_t(){}
	~variable_t(){}
};

struct block_t: public multi_value
{
	friend class ssa_context;
	void push_back( instruction_t* ins );
	void insert( instruction_t* ins, instruction_t* pos );
	bool empty();

	instruction_t* beg;
	instruction_t* end;
	
	std::vector<block_t*>						preds;
	std::vector< std::pair<multi_value*,block_t*> >	succs;
private:
	block_t(){}
	~block_t(){}
};

struct cg_function
{
	sasl::syntax_tree::function_type* fn;

	std::vector<variable_t*> locals;

	multi_value*	retval;	
	block_t*	entry;
	block_t*	exit;
};

END_NS_SASL_SEMANTIC();

#endif