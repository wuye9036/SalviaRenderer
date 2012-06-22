#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/codegen_context.h>
#include <sasl/include/code_generator/llvm/cgs_sisd.h>

#include <eflib/include/platform/typedefs.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/diagnostics/assert.h>

namespace llvm{
	// Node
	class AllocaInst;
	class Argument;
	class Constant;
	class Function;
	class GlobalVariable;
	class Value;
	class BasicBlock;
	
	// Type
	class Type;
	class StructType;

	// Instructions
	class ReturnInst;
}

namespace sasl{
	namespace semantic{
		class symbol;
	}
}

namespace llvm{
	class LLVMContext;
	class Module;
	class ConstantFolder;
	class Type;
	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
	> class IRBuilder;
	typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
		DefaultIRBuilder;
}

BEGIN_NS_SASL_CODE_GENERATOR();

class module_context
{
public:
	static boost::shared_ptr<module_context> create();

	virtual node_context*	get_node_context(sasl::syntax_tree::node*) const = 0;
	virtual node_context*	get_or_create_node_context(sasl::syntax_tree::node*) = 0;
	virtual cg_type*		create_cg_type() = 0;
	virtual function_t*		create_cg_function() = 0;

	virtual llvm::Module*	llvm_module() const = 0;
	virtual llvm::Module*	take_ownership() = 0;
	virtual llvm::DefaultIRBuilder*
							llvm_builder() const = 0;
	virtual llvm::LLVMContext&
							context() const = 0;
	virtual void			dump(std::ostream& ostr) const = 0;

	virtual ~module_context(){}
};

struct node_context
{
	node_context(module_context* owner)
		: owner(owner)
		, function_scope(NULL)
		, is_semantic_mode(false)
		, ty(NULL)
		, declarator_count(0)
	{
	}

	module_context*	owner;
	function_t*		function_scope;		///< Function type.
	value_t			node_value;			///< Value attached to node.
	bool			is_semantic_mode;	///< Expression is a semantic mode. In this mode, the memory get from semantic but not
	cg_type*		ty;					///< Type attached to node.
	insert_point_t	label_position;		///< For labeled statement
	int				declarator_count;	///< The declarator count of declaration.
};

//////////////////////////////////////////////////////////
// Context for SISD.
// It must be an PODs structure.
//
// Remarks:
class cgllvm_sctxt;

struct cgllvm_sctxt_env{
	cgllvm_sctxt_env();

	bool is_semantic_mode;

	bool is_c_compatible;

	insert_point_t continue_to;
	insert_point_t break_to;

	/// Type information used by declarator.
	boost::shared_ptr<cg_type> tyinfo;

	cgllvm_sctxt*			parent_struct;

	llvm::BasicBlock*		block;
	
	/// Current symbol scope.
	sasl::semantic::symbol* sym;

	/// The variable which will pass in initializer to generate initialization code.
	boost::weak_ptr<sasl::syntax_tree::node> variable_to_fill;
};

struct cgllvm_sctxt_data{

	cgllvm_sctxt_data();

	/// Function type.
	function_t self_fn;
	/// Value attached to node.
	value_t val;
	/// Expression is a semantic mode. In this mode, the memory get from semantic but not
	bool semantic_mode;
	/// Type attached to node.
	boost::shared_ptr<cg_type>	tyinfo;
	/// For labeled statement
	insert_point_t position;
	/// The declarator count of declaration.
	int declarator_count;			
};

class cgllvm_sctxt: public codegen_context{
public:
	typedef codegen_context base_type;
	cgllvm_sctxt();

	cgllvm_sctxt( cgllvm_sctxt const& );
	cgllvm_sctxt& operator = ( cgllvm_sctxt const& );

	// Copy all
	void copy( cgllvm_sctxt const* rhs );

	// Copy environment and data
	cgllvm_sctxt_env& env();
	cgllvm_sctxt_env const& env() const;

	void env( cgllvm_sctxt const* rhs );
	void env( cgllvm_sctxt_env const& );

	void clear_data();
	cgllvm_sctxt_data& data();
	cgllvm_sctxt_data const& data() const;

	void data( cgllvm_sctxt_data const& rhs );
	void data( cgllvm_sctxt const* rhs );

	/// @name Accessors.
	/// Expose members in environment and data for easily using.
	/// @{
	cg_type* get_typtr() const;
	boost::shared_ptr<cg_type> get_tysp() const;

	value_t const& value() const;
	value_t& value();

	value_t get_rvalue() const;
	/// @}

private:
	cgllvm_sctxt_data hold_data;
	cgllvm_sctxt_env hold_env;
};

END_NS_SASL_CODE_GENERATOR();

#endif