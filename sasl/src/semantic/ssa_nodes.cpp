#include <sasl/include/semantic/ssa_nodes.h>

#include <cassert>

BEGIN_NS_SASL_SEMANTIC();

void block_t::push_back( instruction_t* ins ){
	insert( ins, end );
}

void block_t::insert( instruction_t* ins, instruction_t* pos )
{
	assert( !ins->parent );
	assert( pos->parent == this );

	ins->next = pos;
	ins->prev = pos->prev;
	pos->prev = ins;
	
	if( ins->prev ){
		ins->prev->next = ins;
	} else {
		beg = ins;
	}
}

END_NS_SASL_SEMANTIC();