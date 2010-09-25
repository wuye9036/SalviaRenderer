#include <sasl/include/semantic/type_converter.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using namespace ::sasl::syntax_tree;
using namespace ::std;

void type_converter::register_converter(
	conv_type ct,
	boost::shared_ptr<type_specifier> src_type,
	boost::shared_ptr<type_specifier> dest_type,
	converter_t conv
	)
{
	convinfos.push_back( make_tuple( ct, src_type, dest_type, conv ) );
}

type_converter::conv_type type_converter::convert( boost::shared_ptr<node> dest, boost::shared_ptr<node> src ){
	boost::shared_ptr<type_specifier> dst_type = extract_semantic_info<type_info_si>( dest )->type_info();
	boost::shared_ptr<type_specifier> src_type = extract_semantic_info<type_info_si>( src )->type_info();

	conv_type ret_ct = cannot_conv;
	for( vector<conv_info>::iterator it = convinfos.begin(); it != convinfos.end(); ++it ){
		if ( type_equal( dst_type, it->get<2>() ) && type_equal( src_type, it->get<1>() ) ){
			ret_ct = it->get<0>();
			// do conversation.
			if( !it->get<3>().empty() ){
				it->get<3>()( dest, src );
			}
		}
	}
	return ret_ct;
}

type_converter::type_converter(){
}

END_NS_SASL_SEMANTIC();