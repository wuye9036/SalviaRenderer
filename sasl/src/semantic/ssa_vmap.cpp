#include <sasl/include/semantic/ssa_vmap.h>

#include <sasl/include/semantic/ssa_nodes.h>
#include <sasl/include/semantic/ssa_context.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::unordered_set;
using std::vector;
using std::pair;
using std::make_pair;

BEGIN_NS_SASL_SEMANTIC();

class vmap_constructor
{
public:
	bool construct( function_t* fn )
	{
		construct_block_vmap( fn->entry );
	}

private:
	void construct_block_vmap( block_t* b ){
		if( processed.count(b) > 0 ){ return; }

		if( b->preds.size() > 1 ){
			BOOST_FOREACH( variable_t* var, vars ){
				instruction_t* phi_inst = ctxt->emit( b, b->beg, instruction_t::phi );

				BOOST_FOREACH( block_t* p, b->preds ){
					if( processed.count(p) > 0 ){
						phi_inst->params.push_back( vmap->load( p->end, var ) );
					} else {
						phi_worklist.push_back( make_pair(p, phi_inst) );
					}
				}

				vmap->store( phi_inst, var, phi_inst );
			}
		}

		for( instruction_t* ins = b->beg; ins != b->end; ins = ins->next ){
			switch( ins->id ){
			case instruction_t::save :
				// Save
				// vmap->store( ins, var, ins->var, ins->params[0] );
				break;
			}
		}

		processed.insert( b );

		/*		
		BOOST_FOREACH( block_t* s, b->succs ){
			construct_block_vmap(s);
		}
		*/
	}
	
	void fix_phi()
	{
		pair<block_t*, instruction_t*> connection;
		BOOST_FOREACH( connection, phi_worklist ){
			block_t*		p	= connection.first;
			instruction_t*	ins	= connection.second;

			BOOST_FOREACH( variable_t* var, vars ){
				ins->params.push_back( vmap->load( p->end, var ) );
			}
		}
	}

	ssa_context*								ctxt;
	unordered_set<block_t*>						processed;
	vector< pair<block_t*, instruction_t*> >	phi_worklist;
	vector< variable_t* >						vars;
	function_vmap*								vmap;
};

void function_vmap::construct_vmap( function_t* /*fn*/ )
{
	// Process all blocks
}

END_NS_SASL_SEMANTIC();