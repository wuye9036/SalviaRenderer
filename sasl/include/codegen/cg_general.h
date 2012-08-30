#ifndef SASL_CODEGEN_CG_GENERAL_H
#define SASL_CODEGEN_CG_GENERAL_H

#include <sasl/include/codegen/forward.h>

#include <sasl/include/codegen/cg_sisd.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/semantic/abi_analyser.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/IRBuilder.h>
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
		class caster_t;
		class module_semantic;
	}
	namespace syntax_tree{
		struct expression;
		struct tynode;
		struct node;
	}
}

namespace llvm{
	class Constant;
}

struct builtin_types;

BEGIN_NS_SASL_CODEGEN();

class cgllvm_sctxt;
class cgllvm_module_impl;
class cgllvm_module;

class cgllvm_general: public cgllvm_sisd{
public:
	typedef cgllvm_sisd parent_class;

	cgllvm_general();

	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( expression_list );

	SASL_VISIT_DCL( identifier );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( tynode );
	SASL_VISIT_DCL( alias_type );

	// statement
protected:
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit	, program );
	SASL_SPECIFIC_VISIT_DCL( bin_logic			, binary_expression );
private:
	cgllvm_module_impl* mod_ptr();
};

END_NS_SASL_CODEGEN()

#endif