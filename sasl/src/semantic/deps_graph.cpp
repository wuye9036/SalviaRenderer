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
void mark_post_order( dom_tree* tree, block_t* b, unordered_set<block_t*>& visited, vector<block_t*>& post_order ){
	if( visited.count(b) > 0 ){
		return;
	}
	visited.insert(b);
	pair<value_t*, block_t*> succ;
	BOOST_FOREACH( succ, b->succs ){
		mark_post_order( tree, succ.second, visited, post_order );
	}
	tree->dom_node(b)->post_order = post_order.size();
	post_order.push_back( b );
}

// K.D. Cooper, 2001
dom_tree_node* intersect_dom( dom_tree_node* n0, dom_tree_node* n1 )
{
	dom_tree_node* finger0 = n0;
	dom_tree_node* finger1 = n1;

	while( finger0 != finger1 ){
		while( finger0->post_order < finger1->post_order ){
			finger0 = finger0->idom;
		}
		while( finger1->post_order < finger0->post_order ){
			finger1 = finger1->idom;
		}
	}

	return finger0;
}

// K.D. Cooper, 2001
void construct_function_dom_tree( dom_tree* tree, function_t* fn )
{
	// Construct reverse post-order
	unordered_set<block_t*> visited;
	vector<block_t*> post_order_nodes;
	mark_post_order( tree, fn->entry, visited, post_order_nodes );
	
	tree->dom_node( fn->entry )->idom = tree->dom_node( fn->entry );
	bool changed(true);
	while( changed ){
		changed = false;
		BOOST_REVERSE_FOREACH( block_t* b, post_order_nodes )
		{
			if( b == fn->entry ){ continue; }
			dom_tree_node* new_idom = NULL;
			BOOST_FOREACH( block_t* p, b->preds ) {
				dom_tree_node* pred_dom_node = tree->dom_node( p );
				if( !new_idom ) {
					if ( pred_dom_node->idom ){ new_idom = pred_dom_node; }
					continue;
				}
				if( pred_dom_node->idom != NULL ){
					new_idom = intersect_dom( pred_dom_node, new_idom );
				}
			}
			dom_tree_node* b_dom_node = tree->dom_node(b);
			if( b_dom_node->idom != new_idom ){
				b_dom_node->idom = new_idom;
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
		construct_function_dom_tree( ret.get(), g->ssa_fn( fn_sym->node().get() ) );
	}

	return ret;
}

dom_tree_node* dom_tree::dom_node( block_t* b )
{
	boost::unordered_map<block_t*, dom_tree_node*>::iterator it;
	it = dom_nodes.find(b);
	if( it != dom_nodes.end() ){
		return it->second;
	}

	dom_tree_node* ret = new dom_tree_node();
	ret->block = b;
	ret->idom = NULL;
	ret->post_order = 0;
	dom_nodes[b] = ret;

	return ret;
}

END_NS_SASL_SEMANTIC();