#ifndef SASL_SEMANTIC_TYPE_CONVERTER_H
#define SASL_SEMANTIC_TYPE_CONVERTER_H

#include <sasl/include/semantic/semantic_forward.h>

#include <sasl/include/semantic/pety.h>

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

class tecov_t{
public:
	enum conv_type{
		better_conv = 0,
		implicit_conv,
		warning_conv,
		explicit_conv,
		cannot_conv = 0xFFFFFFFF
	};
	typedef boost::function<
		void ( boost::shared_ptr< ::sasl::syntax_tree::node >,  boost::shared_ptr< ::sasl::syntax_tree::node >)
	> converter_t;

	tecov_t();
	void register_converter( conv_type ct,
		tid_t /*src*/,
		tid_t /*dest*/,
		converter_t conv );

	conv_type convertible( tid_t dest, tid_t src );
	bool implicit_convertible( tid_t dest, tid_t src );

	void better_or_worse_convertible( tid_t matched, tid_t matching, tid_t src, bool& better, bool& worse );

	conv_type convert( boost::shared_ptr< ::sasl::syntax_tree::node > dest,
		boost::shared_ptr< ::sasl::syntax_tree::node > src );

	conv_type convert( boost::shared_ptr< ::sasl::syntax_tree::tynode > desttype,
		boost::shared_ptr< ::sasl::syntax_tree::node > src );

	virtual ~tecov_t(){}
private:
	typedef boost::tuples::tuple<
		conv_type,
		tid_t,
		tid_t,
		converter_t	> conv_info;
	std::vector< conv_info > convinfos;
};

END_NS_SASL_SEMANTIC();

#endif
