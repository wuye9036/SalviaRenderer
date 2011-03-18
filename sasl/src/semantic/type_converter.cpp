#include <sasl/include/semantic/type_converter.h>

#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using namespace ::sasl::syntax_tree;
using namespace ::std;
using ::boost::shared_ptr;

void type_converter::register_converter(
	conv_type ct,
	type_entry::id_t src_type,
	type_entry::id_t dest_type,
	converter_t conv
	)
{
	convinfos.push_back( make_tuple( ct, src_type, dest_type, conv ) );
}

type_converter::conv_type type_converter::convertible( type_entry::id_t dest, type_entry::id_t src ){
	conv_type ret_ct = cannot_conv;
	for( vector<conv_info>::iterator it = convinfos.begin(); it != convinfos.end(); ++it ){
		if ( dest == it->get<2>() && src == it->get<1>() ){
			ret_ct = it->get<0>();
			break;
		}
	}
	return ret_ct;
}

bool type_converter::implicit_convertible( type_entry::id_t dest, type_entry::id_t src ){
	conv_type ret_ct = cannot_conv;
	for( vector<conv_info>::iterator it = convinfos.begin(); it != convinfos.end(); ++it ){
		if ( dest == it->get<2>() && src == it->get<1>() ){
			ret_ct = it->get<0>();
			break;
		}
	}
	return ret_ct == better_conv || ret_ct == implicit_conv || ret_ct == warning_conv;
}

type_converter::conv_type type_converter::convert( shared_ptr<node> dest, shared_ptr<node> src ){
	type_entry::id_t dst_tid = extract_semantic_info<type_info_si>( dest )->entry_id();
	type_entry::id_t src_tid = extract_semantic_info<type_info_si>( src )->entry_id();

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

type_converter::conv_type type_converter::convert( shared_ptr<type_specifier> desttype, shared_ptr<node> src )
{
	type_entry::id_t dst_tid = extract_semantic_info<type_info_si>( desttype )->entry_id();
	type_entry::id_t src_tid = extract_semantic_info<type_info_si>( src )->entry_id();

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

type_converter::type_converter(){
}

void type_converter::better_or_worse_convertible( type_entry::id_t matched, type_entry::id_t matching, type_entry::id_t src, bool& better, bool& worse )
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