#ifndef SASL_SEMANTIC_SEMANTICS_H
#define SASL_SEMANTIC_SEMANTICS_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/metaprog/util.h>
#include <eflib/include/platform/typedefs.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>
#include <vector>

namespace salviar
{
	class semantic_value;
}

namespace sasl
{
	namespace syntax_tree
	{
		struct node;
		struct labeled_statement;
		EFLIB_DECLARE_STRUCT_SHARED_PTR(program);
	}
	namespace common
	{
		EFLIB_DECLARE_CLASS_SHARED_PTR(diag_chat);
	}
}

BEGIN_NS_SASL_SEMANTIC();

class node_semantic;
class pety_t;
EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);

class module_semantic
{
public:
	module_semantic();
	virtual ~module_semantic(){}
	
	template <typename T> node_semantic* get( boost::shared_ptr<T> const& v )
	{
		node const* pnode = static_cast<node const*>( v.get() );
		return get(*pnode);
	}

	template <typename T> node_semantic* get_or_create( boost::shared_ptr<T> const& v )
	{
		node const* pnode = static_cast<node const*>( v.get() );
		return get_or_create(*pnode);
	}
	
	virtual symbol_ptr						root_symbol() const = 0;
	virtual sasl::syntax_tree::program_ptr	root_program() const = 0;

	virtual pety_t*							pety() const = 0;
	virtual sasl::common::diag_chat_ptr		diags() const = 0;

	virtual std::vector<symbol*> const&	globals() const = 0;
	virtual std::vector<symbol*>&		globals() = 0;

	virtual std::vector<symbol*> const&	functions() const = 0;
	virtual std::vector<symbol*>&		functions() = 0;
	
	virtual std::vector<symbol*> const&	intrinsics() const = 0;
	virtual std::vector<symbol*>&		intrinsics() = 0;

	virtual node_semantic* get( sasl::syntax_tree::node const& ) = 0;
	virtual node_semantic* get_or_create( sasl::syntax_tree::node const& ) = 0;
};

class node_semantic
{
public:
	typedef std::vector<
		boost::weak_ptr<sasl::syntax_tree::labeled_statement>
	>		labeled_statement_array;

	~node_semantic();

	// Read functions
public:
	// General
	module_semantic*
			owner() const { return owner_; }
	sasl::syntax_tree::node*
			associated_node() const { return assoc_node_; }
	symbol*	associated_symbol() const { return assoc_symbol_; }

	// Type
	int		tid() const	{ return tid_; }

	// Expression and variable
	salviar::semantic_value*
			semantic_value() const { return semantic_value_; }
	int		member_index() const { return member_index_; }
	int32_t	swizzle() const {return swizzle_code_; }
	bool	is_reference() const { return is_reference_; }
	bool	is_function_pointer() const { return is_function_pointer_; }

	// Function and intrinsic
	std::string const&
			function_name() const;
	bool	is_intrinsic() const { return is_intrinsic_; }
	bool	is_external() const { return is_external_; }
	bool	msc_compatible() const { return msc_compatible_; }
	bool	is_invoked() const { return is_invoked_; }
	bool	partial_execution() const { return partial_execution_; }
	bool	is_constructor() const { return is_constructor_; }

	// Statement
	labeled_statement_array const&
			labeled_statements() const;
	sasl::syntax_tree::node*
			parent_block() const { return parent_block_; }
	bool	has_loop() const { return has_loop_; }
	
	// Write functions
public:
	// General
	void owner(module_semantic* v) { owner_ = v; }

	void associated_node(sasl::syntax_tree::node* v) { assoc_node_ = v; }
	void associated_symbol(symbol* v) { assoc_symbol_ = v; }

	// Type
	void tid(int v) { tid_ = v; }

	// Expression and variable
	void semantic_value(salviar::semantic_value const& v);

	void member_index(int v) { member_index_ = v; }
	void swizzle(int32_t v) { swizzle_code_ = v; }
	void is_reference(bool v) { is_reference_ = v; }
	void is_function_pointer(bool v) { is_function_pointer_ = v; }

	// Function and intrinsic
	
	void function_name(std::string const& v);
	void is_intrinsic(bool v) { is_intrinsic_ = v; }
	void is_external(bool v) { is_external_ = v; }
	void msc_compatible(bool v) { msc_compatible_ = v; }
	void is_invoked(bool v) { is_invoked_ = v; }
	void partial_execution(bool v) { partial_execution_ = v; }
	void is_constructor(bool v) { is_constructor_ = v; }

	// Statement
	labeled_statement_array& labeled_statements();
	void parent_block(sasl::syntax_tree::node* v) { parent_block_ = v; }
	void has_loop(bool v) { has_loop_ = v; }

private:
	sasl::syntax_tree::node* assoc_node_;
	module_semantic*	owner_;
	symbol*				assoc_symbol_;
	
	// Type
	int 	tid_;
	
	// Expression and variable
	salviar::semantic_value* semantic_value_;
	int		member_index_;
	int32_t	swizzle_code_;
	bool	is_reference_;
	bool	is_function_pointer_;
	
	// Function and intrinsic
	std::string* function_name_;
	bool	is_intrinsic_;
	bool	is_invoked_;
	bool	msc_compatible_;
	bool	is_external_;
	bool	partial_execution_;
	bool	is_constructor_;
	
	// Statement
	labeled_statement_array* labeled_statements_;
	sasl::syntax_tree::node* parent_block_;
	bool	has_loop_;
};

END_NS_SASL_SEMANTIC();

#endif