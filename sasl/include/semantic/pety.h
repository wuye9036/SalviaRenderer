#ifndef SASL_SEMANTIC_TYPE_MANAGER_H
#define SASL_SEMANTIC_TYPE_MANAGER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <eflib/include/platform/typedefs.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl
{
	namespace syntax_tree
	{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
	}
}

struct builtin_types;
struct operators;

BEGIN_NS_SASL_SEMANTIC();

class	module_semantic;
class	node_semantic;
EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);
EFLIB_DECLARE_CLASS_SHARED_PTR(pety_t);

typedef int tid_t;

class pety_t{
public:
	static pety_t_ptr create(module_semantic* owner);

	virtual void	root_symbol(symbol* sym) = 0;

	// Get TID from type informations.
	virtual tid_t	get(const builtin_types& btc) = 0;
	virtual tid_t	get(sasl::syntax_tree::tynode*, symbol*) = 0;
	virtual tid_t	get_array(tid_t elem_type, size_t dimension) = 0;
	virtual tid_t	get_function_type(std::vector<tid_t> const& fn_tids) = 0;
	// Get proto or semantic from TID.
	virtual sasl::syntax_tree::tynode* get_proto(tid_t tid) = 0;
	virtual sasl::syntax_tree::tynode* get_proto_by_builtin(builtin_types bt) = 0;
	virtual void get2(
		tid_t tid,
		sasl::syntax_tree::tynode**,/*output node	 */
		node_semantic**				/*output semantic*/
		) = 0;
	virtual void get2(
		builtin_types btc,
		sasl::syntax_tree::tynode**,/*output node	 */
		node_semantic**				/*output semantic*/
		) = 0;

	// Name mangling
	virtual eflib::fixed_string mangle(
		eflib::fixed_string const&, tid_t tid) = 0;
	virtual eflib::fixed_string operator_name(operators const& op) = 0;

	virtual ~pety_t() {}
};

END_NS_SASL_SEMANTIC();

#endif