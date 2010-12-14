#ifndef SASL_SEMANTIC_TYPE_CONVERTER_H
#define SASL_SEMANTIC_TYPE_CONVERTER_H

#include <sasl/include/semantic/semantic_forward.h>

#include <sasl/include/semantic/type_manager.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/tuple/tuple.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

class type_converter{
public:
	enum conv_type{
		better = 0,
		implicit_conv,
		warning_conv,
		explicit_conv,
		cannot_conv = 0xFFFFFFFF
	};
	typedef boost::function<
		void ( boost::shared_ptr< ::sasl::syntax_tree::node >,  boost::shared_ptr< ::sasl::syntax_tree::node >)
	> converter_t;

	type_converter();
	void register_converter( conv_type ct,
		type_entry::id_t /*src*/,
		type_entry::id_t /*dest*/,
		converter_t conv );

	conv_type convert( boost::shared_ptr< ::sasl::syntax_tree::node > dest,
		boost::shared_ptr< ::sasl::syntax_tree::node > src );\

	conv_type convert( boost::shared_ptr< ::sasl::syntax_tree::type_specifier > desttype,
		boost::shared_ptr< ::sasl::syntax_tree::node > src );

private:
	typedef boost::tuples::tuple<
		conv_type,
		type_entry::id_t,
		type_entry::id_t,
		converter_t	> conv_info;
	std::vector< conv_info > convinfos;
};

END_NS_SASL_SEMANTIC();

#endif
