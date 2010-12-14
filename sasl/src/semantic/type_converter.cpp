#include <sasl/include/semantic/type_converter.h>

#include <sasl/include/semantic/semantic_infos.h>
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

END_NS_SASL_SEMANTIC();