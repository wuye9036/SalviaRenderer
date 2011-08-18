#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/codegen_context.h>
#include <sasl/include/code_generator/llvm/cgllvm_service.h>

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

BEGIN_NS_SASL_CODE_GENERATOR();

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

	/// Type information used by declarator.
	boost::shared_ptr<value_tyinfo> tyinfo;

	cgllvm_sctxt* parent_struct;

	/// If generating code in function, it will be used.
	function_t parent_fn;	

	llvm::BasicBlock* block;
	
	/// Current symbol scope.
	boost::weak_ptr< sasl::semantic::symbol > sym;

	/// The variable which will pass in initilizer to generate initialization code.
	boost::weak_ptr< sasl::syntax_tree::node> variable_to_fill;
};

struct cgllvm_sctxt_data{

	cgllvm_sctxt_data();

	/// Function type.
	function_t self_fn;
	/// Value attached to node.
	value_t val;

	/// Type attached to node.
	boost::shared_ptr<value_tyinfo>	tyinfo;

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
	value_tyinfo* get_typtr() const;
	boost::shared_ptr<value_tyinfo> get_tysp() const;

	value_t const& get_value() const;
	value_t& get_value();

	value_t get_rvalue() const;
	/// @}

private:
	cgllvm_sctxt_data hold_data;
	cgllvm_sctxt_env hold_env;
};

END_NS_SASL_CODE_GENERATOR();

#endif