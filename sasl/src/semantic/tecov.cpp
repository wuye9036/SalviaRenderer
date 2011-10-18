#include <sasl/include/semantic/tecov.h>

#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using namespace ::sasl::syntax_tree;
using namespace ::std;
using ::boost::shared_ptr;

void tecov_t::register_converter(
	conv_type ct,
	tid_t src_type,
	tid_t dest_type,
	converter_t conv
	)
{
	convinfos.push_back( make_tuple( ct, src_type, dest_type, conv ) );
}

tecov_t::conv_type tecov_t::convertible( tid_t dest, tid_t src ){
	conv_type ret_ct = cannot_conv;
	for( vector<conv_info>::iterator it = convinfos.begin(); it != convinfos.end(); ++it ){
		if ( dest == it->get<2>() && src == it->get<1>() ){
			ret_ct = it->get<0>();
			break;
		}
	}
	return ret_ct;
}

bool tecov_t::implicit_convertible( tid_t dest, tid_t src ){
	conv_type ret_ct = cannot_conv;
	for( vector<conv_info>::iterator it = convinfos.begin(); it != convinfos.end(); ++it ){
		if ( dest == it->get<2>() && src == it->get<1>() ){
			ret_ct = it->get<0>();
			break;
		}
	}
	return ret_ct == better_conv || ret_ct == implicit_conv || ret_ct == warning_conv;
}

tecov_t::conv_type tecov_t::convert( shared_ptr<node> dest, shared_ptr<node> src ){
	tid_t dst_tid = extract_semantic_info<type_info_si>( dest )->entry_id();
	tid_t src_tid = extract_semantic_info<type_info_si>( src )->entry_id();

	conv_type ret_ct = cannot_conv;
	for( vector<conv_info>::iterator it = convinfos.begin(); it != convinfos.end(); ++it ){
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

tecov_t::conv_type tecov_t::convert( shared_ptr<tynode> desttype, shared_ptr<node> src )
{
	tid_t dst_tid = desttype->si_ptr<type_info_si>()->entry_id();
	tid_t src_tid = src->si_ptr<type_info_si>()->entry_id();

	conv_type ret_ct = cannot_conv;
	for( vector<conv_info>::iterator it = convinfos.begin(); it != convinfos.end(); ++it ){
		if ( dst_tid == it->get<2>() && src_tid == it->get<1>() ){
			ret_ct = it->get<0>();

			// do conversation.
			if( !it->get<3>().empty() ){
				it->get<3>()( src, src );
			}
		}
	}
	return ret_ct;
}

tecov_t::tecov_t(){
}

void tecov_t::better_or_worse_convertible( tid_t matched, tid_t matching, tid_t src, bool& better, bool& worse )
{
	better = false;
	worse = false;

	if( matching == matched ){
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

	if( convertible( matched, matching ) <= implicit_conv && convertible( matching, src ) <= implicit_conv ){
		better = true;
		return;
	}

	if( convertible(matching, matched)  <= implicit_conv && convertible(matched, src) <= implicit_conv ){
		worse = true;
		return;
	}

	if( convertible(matching, src) < convertible(matched, src) ){
		better = true;
		return;
	}

	if( convertible(matched, src) < convertible(matching, src) ){
		worse = true;
		return;
	}
}

END_NS_SASL_SEMANTIC();