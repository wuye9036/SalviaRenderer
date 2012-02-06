#include <sasl/include/semantic/ssa_context.h>
#include <sasl/include/semantic/ssa_graph.h>
#include <sasl/include/semantic/ssa_nodes.h>

using sasl::syntax_tree::node;
using boost::unordered_map;

BEGIN_NS_SASL_SEMANTIC();

variable_t* ssa_context::create_variable( function_t* parent, node* decl )
{
	variable_t* ret = new variable_t();
	init_value( ret, NULL, value_t::variable, decl );
	ret->fn = parent;
	if( parent ){
		parent->locals.push_back(ret);
	}
	
	vars.push_back( ret );
	return ret;
}

ssa_attribute& ssa_context::attr( node* n )
{
	ssa_attr_iter_t it = ssa_attrs.find(n);
	if( it == ssa_attrs.end() ){
		ssa_attribute* pattr = new ssa_attribute();
		ssa_attrs[n] = pattr;
		return *pattr;
	} else {
		return *(it->second);
	}
}

function_t* ssa_context::create_function()
{
	function_t* ret = new function_t();
	ret->entry = NULL;
	ret->exit = NULL;
	ret->fn = NULL;
	fns.push_back( ret );
	return ret;
}

value_t* ssa_context::create_value( block_t* parent, sasl::syntax_tree::node* attached )
{
	value_t* ret = new value_t();
	init_value( ret, parent, value_t::value, attached );
	values.push_back(ret);
	return ret;
}

block_t* ssa_context::create_block( function_t* parent )
{
	block_t* ret = new block_t();
	init_value( ret, NULL, value_t::block, NULL );
	instruction_t* null_instruction = emit_null();
	null_instruction->parent = ret;
	ret->beg = null_instruction;
	ret->end = null_instruction;
	blocks.push_back(ret);
	return ret;
}

value_t* ssa_context::load( variable_t* var )
{
	// EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

value_t* ssa_context::load( sasl::syntax_tree::node* n )
{
	if( attr(n).var )
	{
		return load( attr(n).var );
	}
	return attr(n).val;
}

void ssa_context::store( variable_t* var, value_t* val )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

instruction_t* ssa_context::emit( block_t* parent, int id )
{
	instruction_t* ret = new instruction_t();
	init_value( ret, NULL, value_t::instruction, NULL );
	ret->id = (instruction_t::IDs)id;
	ret->prev = ret->next = NULL;
	parent->push_back( ret );
	instructions.push_back(ret);
	return ret;
}

void ssa_context::init_value( value_t* v, block_t* parent, int vid, sasl::syntax_tree::node* attached )
{
	v->parent = parent;
	v->attached = attached;
	v->context = this;
	v->vid = (value_t::ID)vid;
}

instruction_t* ssa_context::emit_null()
{
	instruction_t* ret = new instruction_t();
	init_value(ret, NULL, value_t::instruction, NULL);
	ret->prev = NULL;
	ret->next = NULL;
	ret->id = instruction_t::none;
	instructions.push_back(ret);
	return ret;
}

END_NS_SASL_SEMANTIC();

