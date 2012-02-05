#include <sasl/include/semantic/deps_graph.h>

#include <sasl/include/semantic/ssa_context.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using boost::shared_ptr;
using boost::make_shared;
using boost::make_tuple;
using boost::hash_value;
using boost::hash_range;
using boost::hash_combine;
using boost::unordered_set;
using boost::unordered_map;
using std::equal;
using std::make_pair;
using std::vector;
using std::pair;

BEGIN_NS_SASL_SEMANTIC();

shared_ptr<deps_graph> deps_graph::create()
{
	return shared_ptr<deps_graph>( new deps_graph() );
}

void deps_graph::add( address_ident_t const& src, address_ident_t const& dst, dep_kinds dep_kind )
{
	dep_kinds inv_dk = unknown;
	switch( dep_kind ){
	case depends:
		inv_dk = affects;
		break;
	case affects:
		inv_dk = depends;
		break;
	case part_of:
		inv_dk = aggr_of;
		break;
	case aggr_of:
		inv_dk = part_of;
		break;
	default:
		assert(false);
	}

	v2e.insert( make_pair( make_pair( src, dst ), dep_kind ) );
	v2v.insert( make_pair( make_pair( src, dep_kind ), dst ) );
	
	v2e.insert( make_pair( make_pair( dst, src ), inv_dk ) );
	v2v.insert( make_pair( make_pair( dst, inv_dk ), src ) );
}

vector<address_ident_t> deps_graph::inputs_of( address_ident_t const& src ) const
{
	pair<v2v_t::const_iterator, v2v_t::const_iterator> input_it_range = v2v.equal_range( make_pair(src, depends) );
	vector<address_ident_t> ret;
	for( v2v_t::const_iterator it = input_it_range.first; it != input_it_range.second; ++it) {
		ret.push_back( it->second );
	}
	return ret;
}

vector<address_ident_t> deps_graph::outputs_of( address_ident_t const& src ) const
{
	pair<v2v_t::const_iterator, v2v_t::const_iterator> output_it_range = v2v.equal_range( make_pair(src, affects) );
	vector<address_ident_t> ret;
	for( v2v_t::const_iterator it = output_it_range.first; it != output_it_range.second; ++it) {
		ret.push_back( it->second );
	}
	return ret;
}

address_ident_t::address_ident_t( sasl::syntax_tree::node* nd ): agg(nd)
{
}

bool address_ident_t::operator==( address_ident_t const& rhs ) const
{
	if( agg != rhs.agg ) return false;
	if( mem_indexes.size() != rhs.mem_indexes.size() ){ return false; }
	return equal( mem_indexes.begin(), mem_indexes.end(), rhs.mem_indexes.begin() );
}

size_t address_ident_t::hash_value() const
{
	size_t seed = boost::hash_value( agg );
	hash_range( seed, mem_indexes.begin(), mem_indexes.end() );
	return seed;
}

address_ident_t address_ident_t::member_of( size_t index ) const
{
	return address_ident_t( agg, &index, &index+1 );
}

size_t hash_value( address_ident_t const& v )
{
	return v.hash_value();
}


function_t* ssa_graph::ssa_fn( sasl::syntax_tree::node* fn ) const
{
	return ctxt->attr(fn).fn;
}

ssa_context* ssa_graph::context()
{
	if( !ctxt ){
		ctxt = make_shared<ssa_context>();
	}
	return ctxt.get();
}

// K.D. Cooper, 2001
void mark_post_order( dom_tree* tree, block_t* b, unordered_set<block_t*>& visited, vector<block_t*>& post_order, bool is_pdom ){
	if( visited.count(b) > 0 ){
		return;
	}
	
	visited.insert(b);

	if ( !is_pdom ){
		pair<value_t*, block_t*> succ;
		BOOST_FOREACH( succ, b->succs ){
			mark_post_order( tree, succ.second, visited, post_order, is_pdom );
		}
	} else {
		BOOST_FOREACH( block_t* pred, b->preds ){
			mark_post_order( tree, pred, visited, post_order, is_pdom );
		}
	}

	if( is_pdom ){
		tree->dom_node(b)->pdom_post_order = post_order.size();
	} else {
		tree->dom_node(b)->idom_post_order = post_order.size();
	}
	
	post_order.push_back( b );
}

// K.D. Cooper, 2001
dom_tree_node* intersect_dom(
	dom_tree_node* n0,
	dom_tree_node* n1,
	size_t			dom_tree_node::*post_order,
	dom_tree_node*	dom_tree_node::*next_dom 
	)
{
	dom_tree_node* finger0 = n0;
	dom_tree_node* finger1 = n1;

	while( finger0 != finger1 ){
		while( finger0->*post_order < finger1->*post_order ){
			finger0 = finger0->*next_dom;
		}
		while( finger1->*post_order < finger0->*post_order ){
			finger1 = finger1->*next_dom;
		}
	}

	return finger0;
}

// K.D. Cooper, 2001
void construct_function_dom_tree( dom_tree* tree, function_t* fn, bool is_pdom )
{
	// Construct reverse post-order
	unordered_set<block_t*> visited;
	vector<block_t*> post_order_nodes;
	block_t* entry_block = is_pdom ? fn->exit : fn->entry;

	mark_post_order( tree, entry_block, visited, post_order_nodes, is_pdom );
	
	dom_tree_node*	dom_tree_node::*next_dom;
	size_t			dom_tree_node::*post_order;

	next_dom = is_pdom ? &dom_tree_node::pdom : &dom_tree_node::idom;
	post_order = is_pdom ? &dom_tree_node::pdom_post_order : &dom_tree_node::idom_post_order;

	tree->dom_node( entry_block )->*next_dom = tree->dom_node( entry_block );
	bool changed(true);
	while( changed ){
		changed = false;
		BOOST_REVERSE_FOREACH( block_t* b, post_order_nodes )
		{
			if( b == entry_block ){ continue; }
			dom_tree_node* new_dom = NULL;

			vector< pair<value_t*, block_t*> >::iterator succ_iter = b->succs.begin();
			vector<block_t*>::iterator pred_iter = b->preds.begin();
			
			block_t* linked_block = is_pdom ? succ_iter->second : *pred_iter;

			while( linked_block ){
				dom_tree_node* dom_node = tree->dom_node( linked_block );
				if( !new_dom ) {
					if ( dom_node->*next_dom ){ new_dom = dom_node; }
					continue;
				}
				if( dom_node->*next_dom != NULL ){
					new_dom = intersect_dom( dom_node, new_dom, post_order, next_dom );
				}

				linked_block = NULL;
				if( is_pdom ){
					if( ++succ_iter != b->succs.end() ){
						linked_block = succ_iter->second;
					}
				} else {
					if( ++pred_iter != b->preds.end() ){
						linked_block = *pred_iter;
					}
				}
			}

			dom_tree_node* b_dom_node = tree->dom_node(b);
			if( b_dom_node->*next_dom != new_dom ){
				b_dom_node->*next_dom = new_dom;
				changed = true;
			}
		}
	}
}

shared_ptr<dom_tree> dom_tree::construct_dom_tree( module_si* msi, ssa_graph* g )
{
	shared_ptr<dom_tree> ret = make_shared<dom_tree>();

	vector< shared_ptr<symbol> > fns = msi->functions();
	BOOST_FOREACH( shared_ptr<symbol> const& fn_sym, fns )
	{
		construct_function_dom_tree( ret.get(), g->ssa_fn( fn_sym->node().get() ), false );
		construct_function_dom_tree( ret.get(), g->ssa_fn( fn_sym->node().get() ), true );
	}

	return ret;
}

dom_tree_node* dom_tree::dom_node( block_t* b )
{
	unordered_map<block_t*, dom_tree_node*>::iterator it;
	it = dom_nodes.find(b);
	if( it != dom_nodes.end() ){
		return it->second;
	}

	dom_tree_node* ret = new dom_tree_node();
	ret->block = b;
	ret->idom = ret->pdom = NULL;
	ret->idom_post_order = ret->pdom_post_order = 0;
	dom_nodes[b] = ret;

	return ret;
}

block_t* dom_tree::pidom_block( block_t* b )
{
	return dom_node(b)->pdom->block;
}

block_t* dom_tree::idom_block( block_t* b )
{
	return dom_node(b)->idom->block;
}

bool dom_tree::dominance( block_t* b0, block_t* b1 )
{
	dom_tree_node* cur = dom_node(b1);
	dom_tree_node* b0_node = dom_node(b0);

	while( cur ){
		if( cur == b0_node ){ return true; }
		dom_tree_node* idom_node = cur->idom;
		if( idom_node == cur || !idom_node ){
			return false;
		}
		cur = idom_node;
	}

	return false;
}

bool dom_tree::idominance( block_t* b0, block_t* b1 )
{
	return b0 == b1 || idom_block(b1) == b0;
}

bool dom_tree::post_dominance( block_t* b0, block_t* b1 )
{
	dom_tree_node* cur = dom_node(b1);
	dom_tree_node* b0_node = dom_node(b0);

	while( cur ){
		if( cur == b0_node ){ return true; }
		dom_tree_node* pdom_node = cur->pdom;
		if( pdom_node == cur || !pdom_node ){
			return false;
		}
		cur = pdom_node;
	}

	return false;
}

bool dom_tree::post_idominance( block_t* b0, block_t* b1 )
{
	return b0 == b1 || pidom_block(b1) == b0;
}

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
				value_t* merged = ctxt->create_value();
				merged->ins = phi_inst;

				BOOST_FOREACH( block_t* p, b->preds ){
					if( processed.count(p) > 0 ){
						phi_inst->params.push_back( vmap->load( p->end, var ) );
					} else {
						phi_worklist.push_back( make_pair(p, phi_inst) );
					}
				}

				vmap->store( phi_inst, var, merged );
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

void function_vmap::construct_vmap( function_t* fn )
{
	// Process all blocks

	// 
	

}

enum execution_mode
{
	em_unknown,
	em_single,
	em_multiple
};

void compute_block_abi( dom_tree* tree, function_t* fn ){

	unordered_set<block_t*> visited;
	vector<block_t*> post_order_nodes;
	mark_post_order( tree, fn->entry, visited, post_order_nodes, false );

	unordered_map< value_t*, execution_mode > value_ems;
	unordered_map< block_t*, execution_mode > block_ems;

	// Initialize all Value.SOURCE to UNDEF
	// Initialize all Block.MODE to UNDEF

	block_ems[fn->entry] = em_single;
	bool value_changed = true;

	while( value_changed ){
		BOOST_REVERSE_FOREACH( block_t* b, post_order_nodes ){
			execution_mode em = block_ems[b];
			for( instruction_t* ins = b->beg; ins != b->end; ins = ins->next ){
				/*if( block_ems[b] )
				value_ems[ins->ret] = */
			}

			pair<value_t*, block_t*> succ;
			BOOST_FOREACH( succ, b->succs ){
				execution_mode value_em = value_ems.count(succ.first) > 0 ? value_ems[succ.first] : em_unknown;
				execution_mode dst_succ_em = em_unknown;
				execution_mode cur_succ_em = block_ems.count(succ.second) > 0 ? block_ems[succ.second] : em_unknown;

				block_t* succ_idom = tree->idom_block(succ.second);
				if( tree->post_idominance( succ.second, succ_idom ) ){
					dst_succ_em = block_ems[succ_idom];
				} else {
					if( succ.first ){
						dst_succ_em = (execution_mode)std::max( value_em, cur_succ_em );
						dst_succ_em = (execution_mode)std::max( em, dst_succ_em );
					} else {
						dst_succ_em = em;
					}
				}

				if( dst_succ_em != cur_succ_em ){
					block_ems[succ.second] = dst_succ_em;
					value_changed = true;
				}
			}
		}
	}
	/*
	ENTRY.MODE = 'SIMD'
	
	VALUE_CHANGED = TRUE

	while VALUE_CHANGED
		VALUE_CHANGED = FALSE
		Add ENTRY to BLOCK_QUEUE

		for N in BLOCK_QUEUE
			for V in N.VALUES
				DEST_SOURCE = PROMOTE_SOURCE( V.INS.PARAMS )
				if V.SOURCE != DEST_SOURCE
					VALUE_CHANGED = TRUE
					V.SOURCE = DEST_SOURCE

			if N.SUCCS is single
				SUCC = N.SUCCS[0]
				if SUCC.MODE != n.MODE
					add SUCC to BLOCK_QUEUE
					SUCC.MODE = n.MODE
			else 
				for (COND, SUCC) in n.SUCCS
					if COND.SOURCE = UNIFORM or UNDEF
						DEST_MODE = N.MODE
					else (COND.SOURCE = PER_PIXEL)
						DEST_MODE = SISD
					if SUCC.MODE != DEST_MODE
						add SUCC to BLOCK_QUEUE
	*/
}

void colorize_block()
{
	/*
	
	*/
}


block_t::block_t()
{
	end = new instruction_t();
	end->parent = this;

	beg = end;
}

void block_t::push_back( instruction_t* ins )
{
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


instruction_t::instruction_t()
	:var(NULL), id(instruction_t::none), parent(NULL), prev(NULL), next(NULL)
{

}

END_NS_SASL_SEMANTIC();