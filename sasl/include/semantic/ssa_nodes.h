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
struct function_t;
struct value_t;
struct variable_t;

struct value_t
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
	value_t(){}
	~value_t(){}
};

struct instruction_t: public value_t
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

	std::vector<value_t*>	params;

	instruction_t*	next;
	instruction_t*	prev;
private:
	instruction_t(){}
	~instruction_t(){}
};

struct variable_t: public value_t
{
	friend class ssa_context;
	std::vector<size_t> members;
	function_t*			fn;
private:
	variable_t(){}
	~variable_t(){}
};

struct block_t: public value_t
{
	friend class ssa_context;
	void push_back( instruction_t* ins );
	void insert( instruction_t* ins, instruction_t* pos );
	bool empty();

	instruction_t* beg;
	instruction_t* end;
	
	std::vector<block_t*>						preds;
	std::vector< std::pair<value_t*,block_t*> >	succs;
private:
	block_t(){}
	~block_t(){}
};

struct function_t
{
	sasl::syntax_tree::function_type* fn;

	std::vector<variable_t*> locals;

	value_t*	retval;	
	block_t*	entry;
	block_t*	exit;
};

END_NS_SASL_SEMANTIC();

#endif