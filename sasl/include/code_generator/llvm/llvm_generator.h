#ifndef SASL_CODE_GENERATOR_LLVM_LLVM_GENERATOR_H
#define SASL_CODE_GENERATOR_LLVM_LLVM_GENERATOR_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <boost/shared_ptr.hpp>

BEGIN_NS_SASL_CODE_GENERATOR()

class llvm_code_generator: public sasl::syntax_tree::syntax_tree_visitor{
	llvm_code_generator();

	virtual void visit( sasl::syntax_tree::unary_expression& v );
	virtual void visit( sasl::syntax_tree::cast_expression& v);
	virtual void visit( sasl::syntax_tree::binary_expression& v );
	virtual void visit( sasl::syntax_tree::expression_list& v );
	virtual void visit( sasl::syntax_tree::cond_expression& v );
	virtual void visit( sasl::syntax_tree::index_expression& v );
	virtual void visit( sasl::syntax_tree::call_expression& v );
	virtual void visit( sasl::syntax_tree::member_expression& v );

	virtual void visit( sasl::syntax_tree::constant_expression& v );
	virtual void visit( sasl::syntax_tree::constant& v );
	virtual void visit( sasl::syntax_tree::identifier& v );

	// declaration & type specifier
	virtual void visit( sasl::syntax_tree::initializer& v );
	virtual void visit( sasl::syntax_tree::expression_initializer& v );
	virtual void visit( sasl::syntax_tree::member_initializer& v );
	virtual void visit( sasl::syntax_tree::declaration& v );
	virtual void visit( sasl::syntax_tree::variable_declaration& v );
	virtual void visit( sasl::syntax_tree::type_definition& v );
	virtual void visit( sasl::syntax_tree::type_specifier& v );
	virtual void visit( sasl::syntax_tree::buildin_type& v );
	virtual void visit( sasl::syntax_tree::type_identifier& v );
	virtual void visit( sasl::syntax_tree::qualified_type& v );
	virtual void visit( sasl::syntax_tree::array_type& v );
	virtual void visit( sasl::syntax_tree::struct_type& v );
	virtual void visit( sasl::syntax_tree::parameter& v );
	virtual void visit( sasl::syntax_tree::function_type& v );

	// statement
	virtual void visit( sasl::syntax_tree::statement& v );
	virtual void visit( sasl::syntax_tree::declaration_statement& v );
	virtual void visit( sasl::syntax_tree::if_statement& v );
	virtual void visit( sasl::syntax_tree::while_statement& v );
	virtual void visit( sasl::syntax_tree::dowhile_statement& v );
	virtual void visit( sasl::syntax_tree::case_label& v );
	virtual void visit( sasl::syntax_tree::switch_statement& v );
	virtual void visit( sasl::syntax_tree::compound_statement& v );
	virtual void visit( sasl::syntax_tree::expression_statement& v );
	virtual void visit( sasl::syntax_tree::jump_statement& v );

private:
	llvm::LLVMContext ctxt;

	struct CodeGenerationContext{
		boost::shared_ptr<llvm::Module> mod;
		boost::shared_ptr<llvm::IRBuilder<> > builder;

		llvm::Function* created_function;
		llvm::Value* created_value;
	};

	class symbol_info: public sasl::syntax_tree::symbol_info{
	public:
		typedef sasl::syntax_tree::symbol_info base_type;
		symbol_info()£º base_type("llvm code generator symbol info"){
		}
	private:
		llvm::Value* sym_value;
	};

	CodeGenerationContext cg_ctxt;
};



END_NS_SASL_CODE_GENERATOR()

#endif