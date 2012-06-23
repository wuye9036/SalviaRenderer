#ifndef SASL_SEMANTIC_SYMBOL_H
#define SASL_SEMANTIC_SYMBOL_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/static_assert.hpp>
#include <boost/unordered_map.hpp>
#include <boost/type_traits.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>
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
		struct function_type;
		struct tynode;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using sasl::syntax_tree::function_type;
using sasl::syntax_tree::node;
using sasl::syntax_tree::tynode;

class caster_t;
class module_semantic;

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

	symbol* find		(std::string const& name) const;
	symbol* find_this	(std::string const& mangled) const;
	int		count		(std::string const& name) const;

	symbol_array find_overloads(std::string const& name) const;
	symbol_array find_overloads(std::string const& name, caster_t* conv, expression_array const& args) const;
	symbol_array find_assign_overloads(std::string const& name, caster_t* conv, expression_array const& args) const;
	
	symbol*	add_named_child(std::string const& mangled, node* child_node);
	symbol*	add_child(node* child_node);
	symbol*	add_function_begin(function_type* child_fn );
	bool	add_function_end(symbol* sym);

	void remove_child(std::string const& mangled);
	void remove_child(symbol*);
	void remove();

	symbol* parent() const;

	node* associated_node() const;
	void associated_node(node*); ///< Don't call it as common API. It is reserved for internal class.

	std::string const& unmangled_name() const;
	std::string const& mangled_name() const;

private:
	static symbol* create(module_semantic* owner, symbol* parent, node* assoc_node);
	static symbol* create(module_semantic* owner, symbol* parent, node* assoc_node, std::string const& mangled);

	symbol(module_semantic* owner, symbol* parent, node* assoc_node, std::string const* mangled);

	std::vector<std::string> const& get_overloads(std::string const& umnalged) const;

	symbol_array find_overloads_impl(std::string const& name, caster_t* conv, expression_array const& args) const;
	void collapse_vector1_overloads( symbol_array& candidates ) const;

	typedef boost::unordered_map<node*, symbol*>		children_dict;
	typedef children_dict::iterator						children_dict_iterator;

	typedef boost::unordered_map<std::string, symbol*>	named_children_dict;
	typedef named_children_dict::iterator				named_children_dict_iterator; 

	typedef boost::unordered_map<
		std::string, std::vector<std::string> >			overload_dict;
	
	module_semantic*	owner_;
	node*				associated_node_;
	symbol*				parent_;

	// name
	std::string			mangled_name_;
	std::string			unmangled_name_;

	children_dict		children_;
	named_children_dict	named_children_;
	overload_dict		overloads_;
	std::vector<
		std::string>	null_overloads_;
};

END_NS_SASL_SEMANTIC()

#endif
