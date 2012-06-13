#include <sasl/include/semantic/caster.h>

#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using namespace ::sasl::syntax_tree;
using namespace ::std;
using boost::shared_ptr;
using boost::unordered_map;
using boost::bimap;

void caster_t::add_cast( casts ct, tid_t src, tid_t dest, cast_t conv )
{
	add_cast(ct, std::numeric_limits<int>::max(), src, dest, conv);
}

void caster_t::add_cast( casts ct, int prior, tid_t src, tid_t dest, cast_t conv )
{
	assert( ct==caster_t::eql || ct==caster_t::imp || ct==caster_t::exp );
	cast_infos.push_back( boost::make_tuple(ct, prior, src, dest, conv) );
	cast_info_dict.insert( cast_info_dict_t::value_type(make_pair(src,dest),cast_infos.size()-1) );
	if( ct == eql )
	{
		eql_casts.insert( bimap<tid_t, tid_t>::value_type(src, dest) );
	}
}

void caster_t::add_cast_auto_prior( casts ct, tid_t src, tid_t dest, cast_t conv )
{
	if( ct != caster_t::imp )
	{
		add_cast(ct, src, dest, conv);
		return;
	}

	unordered_map<tid_t,int>::iterator it = lowest_priors.find(src);
	if( it != lowest_priors.end() )
	{
		add_cast(ct, ++(it->second), src, dest, conv);
	}
	else
	{
		add_cast(ct, 1, src, dest, conv);
		lowest_priors.insert( make_pair(src, 1) );
	}
}

caster_t::casts caster_t::try_cast( tid_t dest, tid_t src )
{
	int prior = 0;
	return try_cast(prior, dest, src); 
}

caster_t::casts caster_t::try_cast(int& prior, tid_t dest, tid_t src)
{
	prior = std::numeric_limits<int>::max();

	cast_info const* caster1 = NULL;
	cast_info const* caster2 = NULL;
	tid_t			 imm	= -1;

	cast_info const* major_caster = find_caster( caster1, caster2, imm, dest, src, false );
	if( major_caster )
	{
		prior = major_caster->get<1>();
		return major_caster->get<0>();
	}
	return nocast;
}

bool caster_t::try_implicit( tid_t dest, tid_t src )
{
	casts ret = try_cast(dest, src);
	return ret==eql || ret==imp;
}

caster_t::casts caster_t::cast(shared_ptr<node> dest, shared_ptr<node> src)
{
	return cast( dest.get(), src.get() );
}

caster_t::casts caster_t::cast( sst::node* dest, sst::node* src )
{
	tid_t dst_tid = dest->si_ptr<type_info_si>()->entry_id();
	tid_t src_tid = src->si_ptr<type_info_si>()->entry_id();

	cast_info const* caster1 = NULL;
	cast_info const* caster2 = NULL;
	tid_t			 imm	= -1;

	cast_info const* major_caster = find_caster( caster1, caster2, imm, dst_tid, src_tid, false );

	if(!major_caster){ return nocast; }

	if( imm != -1 )
	{
		tynode* tyn = get_tynode(imm);
		assert(tyn);
		caster1->get<4>()(tyn, src);
		caster2->get<4>()(dest, src);
	}
	else
	{
		major_caster->get<4>()(dest, src);
	}

	return major_caster->get<0>();
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

caster_t::cast_info const* caster_t::find_caster(
	cast_info const*& first_caster, cast_info const*& second_caster,
	tid_t& immediate_tid,
	tid_t dest, tid_t src, bool direct_caster_only
	)
{
	immediate_tid = -1;
	first_caster = second_caster = NULL;

	// SRC --xxx-> DST
	cast_info_dict_t::iterator it = cast_info_dict.find( make_pair(src,dest) );
	if( it != cast_info_dict.end() )
	{
		first_caster = boost::addressof(cast_infos[it->second]);
		return first_caster;
	}

	if( direct_caster_only ){ return NULL; }

	// SRC --eql-> IMM --xxx-> DST
	for(int loop_once = 0; loop_once < 1; ++loop_once)
	{
		// get IMM
		bimap<tid_t, tid_t>::left_const_iterator eqlcasts_it = eql_casts.left.find(src);
		if( eqlcasts_it == eql_casts.left.end() ) { break; }

		tid_t eql_tid = eqlcasts_it->second;

		// SRC --eql-> IMM
		cast_info const* caster_1st = NULL;
		cast_info const* caster_2st = NULL;
		tid_t			 imm_tid = -1;
		find_caster(caster_1st, caster_2st, imm_tid, eql_tid, src, true);
		assert(caster_1st);
		cast_info const* eql_caster = caster_1st;

		// IMM --xxx-> DST
		find_caster(caster_1st, caster_2st, imm_tid, dest, eql_tid, true);
		if(imm_tid != -1) { break; } // Only immediate cast once.
		if(caster_1st == NULL) { break; } // No cast

		first_caster = eql_caster;
		second_caster = caster_1st;
		immediate_tid = eql_tid;
		return second_caster;
	}

	// SRC --xxx-> IMM --eql-> DST
	for(int loop_once = 0; loop_once < 1; ++loop_once)
	{
		// get IMM
		bimap<tid_t, tid_t>::right_iterator eqlcasts_it = eql_casts.right.find(dest);
		if( eqlcasts_it == eql_casts.right.end() ) {
			break;
		}

		tid_t eql_tid = eqlcasts_it->second;

		// IMM --eql-> DST
		cast_info const* caster_1st = NULL;
		cast_info const* caster_2st = NULL;
		tid_t			 imm_tid = -1;
		find_caster(caster_1st, caster_2st, imm_tid, dest, eql_tid, true);
		if(!caster_1st){ return NULL; }
		cast_info const* eql_caster = caster_1st;

		// SRC --xxx-> IMM
		find_caster(caster_1st, caster_2st, imm_tid, eql_tid, src, true);
		if(imm_tid != -1) { break; } // Only immediate cast once.
		if(caster_1st == NULL) { break; } // No cast

		first_caster = caster_1st;
		second_caster = eql_caster;
		immediate_tid = eql_tid;
		return first_caster;
	}

	return NULL;
}

tynode* caster_t::get_tynode( tid_t tid )
{
	assert(tynode_getter);
	return tynode_getter(tid);
}

void caster_t::set_tynode_getter( get_tynode_fn fn )
{
	tynode_getter = fn;
}

END_NS_SASL_SEMANTIC();