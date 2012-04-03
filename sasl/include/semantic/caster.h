#ifndef SASL_SEMANTIC_TYPE_CONVERTER_H
#define SASL_SEMANTIC_TYPE_CONVERTER_H

#include <sasl/include/semantic/semantic_forward.h>

#include <sasl/include/semantic/pety.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

namespace sst = sasl::syntax_tree;

class caster_t{
public:
	enum casts
	{
		eql = 0,
		better,
		imp,
		exp,
		nocast = 0xFFFFFFFF
	};

	typedef boost::function<void (boost::shared_ptr<sst::node>, boost::shared_ptr<sst::node>)> cast_t;

	caster_t();
	void  add_cast(casts ct,			tid_t src, tid_t dest, cast_t conv);
	void  add_cast(casts ct, int prior, tid_t src, tid_t dest, cast_t conv);
	void  add_cast_auto_prior(casts ct, tid_t src, tid_t dest, cast_t conv);

	casts try_cast		( int& prior, tid_t dest, tid_t src );
	casts try_cast		(			  tid_t dest, tid_t src );
	bool  try_implicit	( tid_t dest, tid_t src );

	void better_or_worse( tid_t matched, tid_t matching, tid_t src, bool& better, bool& worse );

	casts cast(boost::shared_ptr<sst::node>   dest,	  boost::shared_ptr<sst::node> src);
	casts cast(boost::shared_ptr<sst::tynode> destty, boost::shared_ptr<sst::node> src);

	virtual ~caster_t(){}
private:
	typedef boost::tuples::tuple<
		casts/*result*/, int/*prior*/,
		tid_t/*src*/, tid_t/*dest*/, cast_t/*caster*/
	> cast_info;
	boost::unordered_map<tid_t,int>	lowest_priors;
	std::vector<cast_info>			cast_infos;
};

END_NS_SASL_SEMANTIC();

#endif
