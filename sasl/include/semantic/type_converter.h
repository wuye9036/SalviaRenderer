#ifndef SASL_SEMANTIC_TYPE_CONVERTER_H
#define SASL_SEMANTIC_TYPE_CONVERTER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/function.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
		struct type_specifier;
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
		void ( boost::shared_ptr<::sasl::syntax_tree::node>,  boost::shared_ptr<::sasl::syntax_tree::node>)
	> converter_t;

	void register_converter( conv_type ct,
		boost::shared_ptr<::sasl::syntax_tree::type_specifier> /*src*/,
		boost::shared_ptr<::sasl::syntax_tree::type_specifier> /*dest*/,
		converter_t conv );

	conv_type convert( boost::shared_ptr<::sasl::syntax_tree::node> dest,
		boost::shared_ptr<::sasl::syntax_tree::node> src );

private:
	typedef boost::tuples::tuple<
		conv_type,
		boost::shared_ptr<::sasl::syntax_tree::type_specifier>,
		boost::shared_ptr<::sasl::syntax_tree::type_specifier>,
		converter_t	> conv_info;
	type_converter();
	std::vector< conv_info > convinfos;
};

END_NS_SASL_SEMANTIC();

#endif