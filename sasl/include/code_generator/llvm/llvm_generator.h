#ifndef SASL_CODE_GENERATOR_LLVM_LLVM_GENERATOR_H
#define SASL_CODE_GENERATOR_LLVM_LLVM_GENERATOR_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/syntax_tree/visitor.h>

BEGIN_NS_SASL_CODE_GENERATOR()

class llvm_code_generator: public sasl::syntax_tree::syntax_tree_visitor{
	virtual void visit( sasl::syntax_tree::unary_expression& v );
	virtual void visit( sasl::syntax_tree::cast_expression& v);
	virtual void visit( sasl::syntax_tree::binary_expression& v );
	virtual void visit( sasl::syntax_tree::expression_list& v );
	virtual void visit( sasl::syntax_tree::cond_expression& v );
	virtual void visit( sasl::syntax_tree::index_expression& v );
	virtual void visit( sasl::syntax_tree::call_expression& v );
	virtual void visit( sasl::syntax_tree::member_expression& v );

	virtual void visit( sasl::syntax_tree::constant_expression& v );
	virtual void visit( sasl::syntax_tree::constant& v ) = 0;
	virtual void visit( sasl::syntax_tree::identifier& v ) = 0;
};

END_NS_SASL_CODE_GENERATOR()

#endif