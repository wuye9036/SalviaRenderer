#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_GENERAL_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_GENERAL_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/semantic/abi_analyser.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace sasl{
	namespace semantic{
		class type_converter;
		class module_si;
	}
	namespace syntax_tree{
		struct expression;
		struct type_specifier;
		struct node;
	}
}

namespace llvm{
	class Constant;
}

struct builtin_type_code;

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_sctxt;
class cgllvm_modimpl;
class llvm_module;

class cgllvm_general: public cgllvm_sisd{
public:
	cgllvm_general();

	SASL_VISIT_DCL( unary_expression );
	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( binary_expression );
	SASL_VISIT_DCL( expression_list );
	SASL_VISIT_DCL( cond_expression );
	SASL_VISIT_DCL( index_expression );
	SASL_VISIT_DCL( call_expression );
	SASL_VISIT_DCL( member_expression );

	SASL_VISIT_DCL( constant_expression );
	SASL_VISIT_DCL( variable_expression );
	SASL_VISIT_DCL( identifier );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	SASL_VISIT_DCL( expression_initializer );
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( declaration );
	SASL_VISIT_DCL( declarator );
	SASL_VISIT_DCL( variable_declaration );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( type_specifier );
	SASL_VISIT_DCL( builtin_type );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( struct_type );
	SASL_VISIT_DCL( parameter );
	SASL_VISIT_DCL( function_type );

	// statement
	SASL_VISIT_DCL( statement );
	SASL_VISIT_DCL( declaration_statement );
	SASL_VISIT_DCL( if_statement );
	SASL_VISIT_DCL( while_statement );
	SASL_VISIT_DCL( dowhile_statement );
	SASL_VISIT_DCL( for_statement );
	SASL_VISIT_DCL( case_label );
	SASL_VISIT_DCL( ident_label );
	SASL_VISIT_DCL( switch_statement );
	SASL_VISIT_DCL( compound_statement );
	SASL_VISIT_DCL( expression_statement );
	SASL_VISIT_DCL( jump_statement );

	// program
	SASL_VISIT_DCL( program );

private:
	cgllvm_modimpl* mod_ptr();
	boost::function<cgllvm_sctxt*( boost::shared_ptr<sasl::syntax_tree::node> const& )> ctxt_getter;

	void do_assign(
		boost::any* data,
		boost::shared_ptr<sasl::syntax_tree::expression> lexpr,
		boost::shared_ptr<sasl::syntax_tree::expression> rexpr
		);

	boost::shared_ptr< ::sasl::semantic::type_converter > typeconv;
};

END_NS_SASL_CODE_GENERATOR()

#endif