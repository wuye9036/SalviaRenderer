#include <sasl/include/semantic/caster.h>

#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using namespace ::sasl::syntax_tree;
using namespace ::std;
using boost::shared_ptr;
using boost::unordered_map;

void caster_t::add_cast( casts ct, tid_t src, tid_t dest, cast_t conv
	)
{
	add_cast(ct, std::numeric_limits<int>::max(), src, dest, conv);
}

void caster_t::add_cast( casts ct, int prior, tid_t src, tid_t dest, cast_t conv )
{
	assert( ct==caster_t::eql || ct==caster_t::imp || ct==caster_t::exp );
	cast_infos.push_back( boost::make_tuple(ct, prior, src, dest, conv) );
}

void caster_t::add_cast_auto_prior( casts ct, tid_t src, tid_t dest, cast_t conv )
{
	if( conv != caster_t::imp )
	{
		add_cast(ct, src, dest, conv);
		return;
	}

	unordered_map<tid_t,int>::iterator it = lowest_priors.find(src);
	if( it != lowest_priors.end() )
	{
		add_cast(ct, (it->second)++, src, dest, conv);
	}
	else
	{
		add_cast(ct, 1, src, dest, conv);
		lowest_priors.insert( make_pair(src, 1) );
	}
}

caster_t::casts caster_t::try_cast( tid_t dest, tid_t src )
{
	int prior;
	return try_cast(prior, dest, src); 
}

caster_t::casts caster_t::try_cast(int& prior, tid_t dest, tid_t src)
{
	casts ret_ct = nocast;
	for( vector<cast_info>::iterator it = cast_infos.begin(); it != cast_infos.end(); ++it )
	{
		if ( dest == it->get<3>() && src == it->get<2>() )
		{
			ret_ct	= it->get<0>();
			prior	= it->get<1>();
			break;
		}
	}
	return ret_ct;
}

bool caster_t::try_implicit( tid_t dest, tid_t src )
{
	casts ret = try_cast(dest, src);
	return ret==eql || ret==better || ret==imp;
}

caster_t::casts caster_t::cast(shared_ptr<node> dest, shared_ptr<node> src)
{
	tid_t dst_tid = dest->si_ptr<type_info_si>()->entry_id();
	tid_t src_tid = dest->si_ptr<type_info_si>()->entry_id();

	casts ret_ct = nocast;
	for( vector<cast_info>::iterator it = cast_infos.begin(); it != cast_infos.end(); ++it )
	{
		if ( dst_tid == it->get<3>() && src_tid == it->get<2>() ){
			ret_ct = it->get<0>();
			// do conversation.
			if( !it->get<4>().empty() )
			{
				it->get<4>()( dest, src );
			}
			break;
		}
	}
	return ret_ct;
}

caster_t::casts caster_t::cast(shared_ptr<tynode> desttype, shared_ptr<node> src)
{
	tid_t dst_tid = desttype->si_ptr<type_info_si>()->entry_id();
	tid_t src_tid = src->si_ptr<type_info_si>()->entry_id();

	casts ret_ct = nocast;
	for( vector<cast_info>::iterator it = cast_infos.begin(); it != cast_infos.end(); ++it )
	{
		if ( dst_tid == it->get<3>() && src_tid == it->get<2>() )
		{
			ret_ct = it->get<0>();
			if( !it->get<4>().empty() )
			{
				it->get<4>()( desttype, src );
			}
		}
	}
	return ret_ct;
}

caster_t::caster_t() {}

void caster_t::better_or_worse( tid_t matched, tid_t matching, tid_t src, bool& better, bool& worse )
{
	better = false;
	worse = false;

	if( matching == matched )
	{
		return;
	}

	if( src == matching )
	{
		better = true;
		return;
	}

	if( src == matched )
	{
		worse = true;
		return;
	}

	if( try_cast(matched, matching)==caster_t::eql )
	{
		return;
	}

	int matched_prior  = std::numeric_limits<int>::max();
	int matching_prior = std::numeric_limits<int>::max();

	casts matching_result	= try_cast(matching_prior, matching, src);
	casts matched_result	= try_cast(matched_prior,  matched,  src);
	
	better = (matching_result < matched_result);
	worse  = (matching_result > matched_result);

	if( better || worse ) { return; }
	
	better = matching_prior < matched_prior;
	worse  = matching_prior > matched_prior;
}

END_NS_SASL_SEMANTIC();