#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H

#include <sasl/include/code_generator/forward.h>

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

	virtual node_context*	get_node_context(sasl::syntax_tree::node const*) const = 0;
	virtual node_context*	get_or_create_node_context(sasl::syntax_tree::node const*) = 0;
	virtual cg_type*		create_cg_type() = 0;
	virtual function_t*		create_cg_function() = 0;

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

END_NS_SASL_CODE_GENERATOR();

#endif