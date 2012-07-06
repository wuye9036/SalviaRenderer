#ifndef SASL_SEMANTIC_TYPE_MANAGER_H
#define SASL_SEMANTIC_TYPE_MANAGER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <eflib/include/platform/typedefs.h>

#include <eflib/include/utility/shared_declaration.h>

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
class node_semantic;
typedef int tid_t;

EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);

class pety_item_t{
public:
	typedef int pety_item_t::*id_ptr_t;
	pety_item_t();
	~pety_item_t();
	sasl::syntax_tree::tynode_ptr	stored;
	node_semantic*					ty_sem;

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

	void get2(tid_t tid,
		sasl::syntax_tree::tynode**, node_semantic**);
	void get2(builtin_types btc,
		sasl::syntax_tree::tynode**, node_semantic**);

	~pety_t();

private:
	/// Add builtin type to pety.
	void add_builtin_type	(
		builtin_types btc,
		node_semantic** out_sem, tid_t* out_tid
		);

	void add_proto_tynode	(
		sasl::syntax_tree::tynode_ptr const& proto_node,
		node_semantic** out_sem, tid_t* out_tid
		);

	void add_tynode			(
		sasl::syntax_tree::tynode* tyn,
		bool attach_tid_to_input,
		node_semantic** out_sem, tid_t* out_tid
		);

	tid_t get(sasl::syntax_tree::tynode* v, symbol* scope, bool attach_tid_to_input);

	typedef boost::unordered_map<builtin_types, tid_t>				bt_dict;
	typedef boost::unordered_map<sasl::syntax_tree::tynode*, tid_t>	tynode_dict;

	bt_dict						bt_dict_;
	tynode_dict					tynode_dict_;
	std::vector<pety_item_t>	type_items_;
	symbol*						root_symbol_;
	module_semantic*			owner_;
};

END_NS_SASL_SEMANTIC();

#endif