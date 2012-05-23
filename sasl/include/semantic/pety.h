#ifndef SASL_SEMANTIC_TYPE_MANAGER_H
#define SASL_SEMANTIC_TYPE_MANAGER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <eflib/include/platform/typedefs.h>

#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace syntax_tree{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
	}
}

struct builtin_types;

BEGIN_NS_SASL_SEMANTIC();

typedef int tid_t;

EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);

class pety_item_t{
public:
	typedef int pety_item_t::*id_ptr_t;
	pety_item_t();
	
	sasl::syntax_tree::tynode_ptr stored;
	tid_t u_qual;
	tid_t a_qual;
};

EFLIB_DECLARE_CLASS_SHARED_PTR(pety_t);
class pety_t{
public:
	static pety_t_ptr create();

	void root_symbol( symbol_ptr const& sym );

	pety_t_ptr handle() const;
	
	tid_t get(sasl::syntax_tree::tynode_ptr const& node, symbol_ptr const& parent);
	tid_t get(const builtin_types& btc);
	sasl::syntax_tree::tynode_ptr get(tid_t id);

	tid_t get_array(tid_t elem_type, size_t dimension);
private:
	tid_t allocate_and_assign_id(sasl::syntax_tree::tynode_ptr const& node);
	std::vector<pety_item_t>	entries;
	boost::weak_ptr<pety_t>		self_handle;
	boost::weak_ptr<symbol>		rootsym;
};

END_NS_SASL_SEMANTIC();

#endif