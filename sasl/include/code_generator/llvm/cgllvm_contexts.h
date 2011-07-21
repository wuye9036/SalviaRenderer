#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/codegen_context.h>

#include <eflib/include/platform/typedefs.h>

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

	llvm::Type const* declarator_type;
	bool is_mat, as_vec;

	cgllvm_sctxt* parent_struct;

	llvm::Function* parent_fn;	// If generating code in function, it will be used.
	llvm::BasicBlock* block;
	
	boost::weak_ptr< sasl::semantic::symbol > sym;
	boost::weak_ptr< sasl::syntax_tree::node> variable_to_fill;
};

struct cgllvm_sctxt_data{
	cgllvm_sctxt_data();

	// Storage
	// Only one of them is avaliable

	// Treated as reference
	// If it is true,
	// Value must stored the address of value of val_type.
	// e.g.
	//  val_type = int
	//	value = 0xDEADBEEF
	//  is_ref = true
	// load() = *(int*)value;
	// *(int*)value = store()
	bool is_ref;
	llvm::Value* val;					// Argument and constant
	llvm::GlobalVariable* global;
	llvm::AllocaInst* local;
	struct aggregated_data{
		cgllvm_sctxt* parent;
		union{
			int index;
			int32_t swizzle;
		};
		bool is_swizzle;
	} agg;
	
	char const* hint_name;

	// Functions
	llvm::Function* self_fn;		///< used by function type.

	bool as_vector;					///< Treated data as vector. Available only if val_type is struct.
	bool is_matrix;					///< Data is built-in matrix.
	llvm::Type const* val_type;		///< Value Type.
	bool is_signed;					///< For integral only.
	int declarator_count;			///< For declaration only
	llvm::ReturnInst* return_inst;	///< Instructions
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

	// Copy some special members
	void storage( cgllvm_sctxt const* rhs );
	void type( cgllvm_sctxt const* rhs );
	void storage_and_type( cgllvm_sctxt* rhs );

private:
	cgllvm_sctxt_data hold_data;
	cgllvm_sctxt_env hold_env;
};

END_NS_SASL_CODE_GENERATOR();

#endif