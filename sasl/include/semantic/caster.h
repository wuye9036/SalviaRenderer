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

class caster_t{
public:
	enum casts{
		eql = 0,
		better,
		imp,
		warning,
		exp,
		nocast = 0xFFFFFFFF
	};
	typedef boost::function<
		void ( boost::shared_ptr< ::sasl::syntax_tree::node >,  boost::shared_ptr< ::sasl::syntax_tree::node >)
	> cast_t;

	caster_t();
	void add_cast( casts ct, tid_t src, tid_t dest, cast_t conv );

	casts try_cast( tid_t dest, tid_t src );
	bool try_implicit( tid_t dest, tid_t src );

	void better_or_worse( tid_t matched, tid_t matching, tid_t src, bool& better, bool& worse );

	casts cast( boost::shared_ptr< ::sasl::syntax_tree::node > dest,
		boost::shared_ptr< ::sasl::syntax_tree::node > src );

	casts cast( boost::shared_ptr< ::sasl::syntax_tree::tynode > desttype,
		boost::shared_ptr< ::sasl::syntax_tree::node > src );

	virtual ~caster_t(){}
private:
	typedef boost::tuples::tuple< casts, tid_t, tid_t, cast_t > cast_info;
	std::vector< cast_info > cast_infos;
};

END_NS_SASL_SEMANTIC();

#endif
