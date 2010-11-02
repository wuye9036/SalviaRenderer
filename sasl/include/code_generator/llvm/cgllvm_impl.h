#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_IMPL_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_IMPL_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/syntax_tree/visitor.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

#include <boost/shared_ptr.hpp>
#include <string>

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_global_context;
class llvm_code;

class llvm_code_generator: public sasl::syntax_tree::syntax_tree_visitor{
public:
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
	virtual void visit( sasl::syntax_tree::variable_expression& v );
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
	virtual void visit( sasl::syntax_tree::for_statement& v );
	virtual void visit( sasl::syntax_tree::case_label& v );
	virtual void visit( sasl::syntax_tree::ident_label& v );
	virtual void visit( sasl::syntax_tree::switch_statement& v );
	virtual void visit( sasl::syntax_tree::compound_statement& v );
	virtual void visit( sasl::syntax_tree::expression_statement& v );
	virtual void visit( sasl::syntax_tree::jump_statement& v );
	
	// program
	virtual void visit( sasl::syntax_tree::program& v );

	boost::shared_ptr<llvm_code> generated_module();
private:
	boost::shared_ptr<cgllvm_global_context> ctxt;
};

END_NS_SASL_CODE_GENERATOR()

#endif