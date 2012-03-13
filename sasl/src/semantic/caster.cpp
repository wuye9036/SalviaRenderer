#include <sasl/include/semantic/caster.h>

#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using namespace ::sasl::syntax_tree;
using namespace ::std;
using ::boost::shared_ptr;

void caster_t::add_cast( casts ct, tid_t src, tid_t dest, cast_t conv
	)
{
	cast_infos.push_back( boost::make_tuple( ct, src, dest, conv ) );
}

caster_t::casts caster_t::try_cast( tid_t dest, tid_t src ){
	casts ret_ct = nocast;
	for( vector<cast_info>::iterator it = cast_infos.begin(); it != cast_infos.end(); ++it ){
		if ( dest == it->get<2>() && src == it->get<1>() ){
			ret_ct = it->get<0>();
			break;
		}
	}
	return ret_ct;
}

bool caster_t::try_implicit( tid_t dest, tid_t src ){
	casts ret_ct = nocast;
	for( vector<cast_info>::iterator it = cast_infos.begin(); it != cast_infos.end(); ++it ){
		if ( dest == it->get<2>() && src == it->get<1>() ){
			ret_ct = it->get<0>();
			break;
		}
	}
	return ret_ct == eql || ret_ct == better || ret_ct == imp || ret_ct == warning;
}

caster_t::casts caster_t::cast( shared_ptr<node> dest, shared_ptr<node> src ){
	tid_t dst_tid = extract_semantic_info<type_info_si>( dest )->entry_id();
	tid_t src_tid = extract_semantic_info<type_info_si>( src )->entry_id();

	casts ret_ct = nocast;
	for( vector<cast_info>::iterator it = cast_infos.begin(); it != cast_infos.end(); ++it ){
		if ( dst_tid == it->get<2>() && src_tid == it->get<1>() ){
			ret_ct = it->get<0>();
			// do conversation.
			if( !it->get<3>().empty() ){
				it->get<3>()( dest, src );
			}
			break;
		}
	}
	return ret_ct;
}

caster_t::casts caster_t::cast( shared_ptr<tynode> desttype, shared_ptr<node> src )
{
	tid_t dst_tid = desttype->si_ptr<type_info_si>()->entry_id();
	tid_t src_tid = src->si_ptr<type_info_si>()->entry_id();

	casts ret_ct = nocast;
	for( vector<cast_info>::iterator it = cast_infos.begin(); it != cast_infos.end(); ++it ){
		if ( dst_tid == it->get<2>() && src_tid == it->get<1>() ){
			ret_ct = it->get<0>();

			// do conversation.
			if( !it->get<3>().empty() ){
				it->get<3>()( desttype, src );
			}
		}
	}
	return ret_ct;
}

caster_t::caster_t(){
}

void caster_t::better_or_worse( tid_t matched, tid_t matching, tid_t src, bool& better, bool& worse )
{
	better = false;
	worse = false;

	if( matching == matched || try_cast( matched, matching ) == caster_t::eql ){
		better = false;
		worse = false;
		return;
	}

	if( src == matching ){
		better = true;
		return;
	}

	if( src == matched ){
		worse = true;
		return;
	}

	if( try_cast( matched, matching ) <= imp && try_cast( matching, src ) <= imp ){
		better = true;
		return;
	}

	if( try_cast(matching, matched)  <= imp && try_cast(matched, src) <= imp ){
		worse = true;
		return;
	}

	if( try_cast(matching, src) < try_cast(matched, src) ){
		better = true;
		return;
	}

	if( try_cast(matched, src) < try_cast(matching, src) ){
		worse = true;
		return;
	}
}

END_NS_SASL_SEMANTIC();