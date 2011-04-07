#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/codegen_context.h>

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
struct cgllvm_sctxt_data{
	cgllvm_sctxt_data();

	// Storage
	// Only one of them is avaliable
	bool is_ref;						// Treated as reference
	llvm::Value* val;					// Argument and constant
	llvm::GlobalVariable* global;
	llvm::AllocaInst* local;
	struct aggregated_data{
		cgllvm_sctxt_data* parent_data;
		int index;
	} agg;

	// Functions
	llvm::Function* parent_fn;	// If generating code in function, it will be used.
	llvm::Function* self_fn;	// used by function type.

	// Code blocks
	llvm::BasicBlock* block;

	llvm::BasicBlock* continue_to;
	llvm::BasicBlock* break_to;

	// Types
	llvm::Type const* val_type;
	bool is_signed;							// For integral only.

	// Instructions
	llvm::ReturnInst* return_inst;
};

class cgllvm_sctxt: public codegen_context{
public:
	typedef codegen_context base_type;
	cgllvm_sctxt();

	boost::weak_ptr< sasl::semantic::symbol > sym;
	boost::weak_ptr< sasl::syntax_tree::node> variable_to_fill;

	cgllvm_sctxt_data& data();
	cgllvm_sctxt_data const& data() const;

	void set_storage( cgllvm_sctxt const* rhs );
	void set_type( cgllvm_sctxt const* rhs );
	void set_storage_and_type( cgllvm_sctxt* rhs );

private:
	cgllvm_sctxt_data hold_data;
};

END_NS_SASL_CODE_GENERATOR();

#endif