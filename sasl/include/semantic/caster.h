#ifndef SASL_SEMANTIC_TYPE_CONVERTER_H
#define SASL_SEMANTIC_TYPE_CONVERTER_H

#include <sasl/include/semantic/semantic_forward.h>

#include <sasl/include/semantic/pety.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bimap.hpp>
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

class node_semantic;

typedef boost::function< sst::tynode* (tid_t) >
	get_tynode_fn;
typedef boost::function< node_semantic* (sst::node*) >
	get_semantic_fn;

class caster_t{
public:
	enum casts
	{
		eql = 0,
		imp,
		exp,
		nocast = 0xFFFFFFFF
	};

	typedef boost::function<void (sst::node*, sst::node*)> cast_t;

	caster_t();

	void  add_cast(casts ct,			tid_t src, tid_t dest, cast_t conv);
	void  add_cast(casts ct, int prior, tid_t src, tid_t dest, cast_t conv);
	void  add_cast_auto_prior(casts ct, tid_t src, tid_t dest, cast_t conv);

	casts try_cast		( int& prior, tid_t dest, tid_t src );
	casts try_cast		(			  tid_t dest, tid_t src );
	bool  try_implicit	( tid_t dest, tid_t src );

	void better_or_worse( tid_t matched, tid_t matching, tid_t src, bool& better, bool& worse );

	casts cast(boost::shared_ptr<sst::node> dest, boost::shared_ptr<sst::node> src);
	casts cast(sst::node* dest, sst::node* src);

	void set_function_get_tynode(get_tynode_fn fn);
	void set_function_get_semantic(get_semantic_fn fn);

	virtual ~caster_t(){}
private:
	typedef boost::tuples::tuple<
		casts/*result*/, int/*prior*/,
		tid_t/*src*/, tid_t/*dest*/, cast_t/*caster*/
	> cast_info;

	typedef boost::unordered_map<
		std::pair<tid_t /*src*/, tid_t /*dest*/>, size_t /*cast info index*/
	> cast_info_dict_t;

	cast_info const* find_caster(
		cast_info const*& first_caster, cast_info const*& second_caster,
		tid_t& immediate_tid,
		tid_t dest, tid_t src, bool direct_caster_only
		); // return non-equal caster.

	boost::unordered_map<tid_t,int>	lowest_priors; // For auto cast priority.
	std::vector<cast_info>			cast_infos;
	cast_info_dict_t				cast_info_dict;
	boost::bimap<tid_t, tid_t>		eql_casts;

	// Functions
	get_tynode_fn					get_tynode_;
	get_semantic_fn					get_semantic_;
};

END_NS_SASL_SEMANTIC();

#endif
