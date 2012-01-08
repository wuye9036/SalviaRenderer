#include <sasl/include/semantic/deps_graph.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <boost/tuple/tuple.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using boost::shared_ptr;
using boost::make_shared;
using boost::make_tuple;
using boost::hash_value;
using boost::hash_range;
using boost::hash_combine;
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

size_t hash_value( address_ident_t const& v )
{
	return v.hash_value();
}

END_NS_SASL_SEMANTIC();