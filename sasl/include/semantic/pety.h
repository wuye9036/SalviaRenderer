#ifndef SASL_SEMANTIC_TYPE_MANAGER_H
#define SASL_SEMANTIC_TYPE_MANAGER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <eflib/include/platform/typedefs.h>

#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace syntax_tree{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
	}
}

struct builtin_types;

BEGIN_NS_SASL_SEMANTIC();

class module_semantic;
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
	static pety_t_ptr create( module_semantic* owner );

	void root_symbol(symbol* sym);

	tid_t get(const builtin_types& btc);
	tid_t get(sasl::syntax_tree::tynode*, symbol*);
	tid_t get_array(tid_t elem_type, size_t dimension);

	sasl::syntax_tree::tynode* get_proto(tid_t tid);
	sasl::syntax_tree::tynode* get_proto_by_builtin(builtin_types bt);

private:
	tid_t allocate_and_assign_id(sasl::syntax_tree::tynode* node);

	boost::unordered_map<builtin_types, tid_t>
								bt_dict_;
	boost::unordered_map<sasl::syntax_tree::tynode*, tid_t>
								tynode_dict_;
	std::vector<pety_item_t>	type_items_;
	symbol*						root_symbol_;
	module_semantic*			owner_;
};

END_NS_SASL_SEMANTIC();

#endif