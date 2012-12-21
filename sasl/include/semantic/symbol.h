#ifndef SASL_SEMANTIC_SYMBOL_H
#define SASL_SEMANTIC_SYMBOL_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/static_assert.hpp>
#include <boost/unordered_map.hpp>
#include <boost/type_traits.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl
{
	namespace common
	{
		class diag_chat;
	}
	namespace syntax_tree
	{
		struct node;
		struct expression;
		struct function_def;
		struct tynode;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using sasl::syntax_tree::function_def;
using sasl::syntax_tree::node;
using sasl::syntax_tree::tynode;

class caster_t;
class module_semantic;

typedef int tid_t;

//////////////////////////////////////////////////////////////////////////
/*
	In sasl, symbol can assigned to any syntax node. But in fact, only some
	sorts of node make senses, such as variable declaration, function and
	struct declaration/definition, type re-definition, statement.

	Every symbol has three names.
	* THE FIRST NAME is "unmangled name", briefly "name". It means
	the literal name of syntax node. For a example, a variable name or a label.
	* THE SECOND NAME is "mangled name". It is useful for function overloading.
	There is a document describing its details.
	* THE THIRD NAME is "full path". It is a compile unit unique name or
	a target unique name, even a global unique name. Some external interface
	may use it.
*/
//////////////////////////////////////////////////////////////////////////
class symbol{
public:
	typedef std::vector<sasl::syntax_tree::expression*> expression_array;
	typedef std::vector<symbol*>	symbol_array;
	typedef symbol_array::iterator	symbol_array_iterator;

	static symbol* create_root(module_semantic* owner, node* root_node = NULL);

	module_semantic* owner() const;

	symbol* find		(eflib::fixed_string const& name) const;
	symbol* find_this	(eflib::fixed_string const& mangled) const;
	int		count		(eflib::fixed_string const& name) const;

	symbol_array find_overloads(eflib::fixed_string const& name) const;
	symbol_array find_overloads(eflib::fixed_string const& name, caster_t* conv, expression_array const& args) const;
	symbol_array find_assign_overloads(eflib::fixed_string const& name, caster_t* conv, expression_array const& args) const;
	
	symbol*	add_named_child(eflib::fixed_string const& mangled, node* child_node);
	symbol*	add_child(node* child_node);
	symbol*	add_function_begin(function_def* child_fn);
	bool	add_function_end(symbol* sym, tid_t fn_tid);
	void	cancel_function(symbol* sym);

	void remove_child(symbol*);
	void remove();

	symbol* parent() const;

	node* associated_node() const;
	void associated_node(node*); ///< Don't call it as common API. It is reserved for internal class.

	eflib::fixed_string const& unmangled_name() const;
	eflib::fixed_string const& mangled_name() const;

private:
	static symbol* create(module_semantic* owner, symbol* parent, node* assoc_node);
	static symbol* create(module_semantic* owner, symbol* parent, node* assoc_node, eflib::fixed_string const& mangled);
	
	symbol(module_semantic* owner, symbol* parent, node* assoc_node, eflib::fixed_string const* mangled);

	symbol_array find_overloads_impl(eflib::fixed_string const& name, caster_t* conv, expression_array const& args) const;
	void collapse_vector1_overloads( symbol_array& candidates ) const;

	typedef boost::unordered_map
		<node*, symbol*>								children_dict;
	typedef boost::unordered_map<
		eflib::fixed_string, symbol*>					named_children_dict;
	typedef boost::unordered_map<eflib::fixed_string,
		std::pair<symbol_array, std::vector<tid_t> > >	overload_dict;
	
	module_semantic*			owner_;
	node*						associated_node_;
	symbol*						parent_;

	// name
	eflib::fixed_string			mangled_name_;
	eflib::fixed_string			unmangled_name_;

	children_dict				children_;
	named_children_dict			named_children_;
	overload_dict				overloads_;
	std::vector<
		eflib::fixed_string>	null_overloads_;

	static eflib::fixed_string  null_name;
};

END_NS_SASL_SEMANTIC()

#endif
