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
		class caster_t;
		class module_si;
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

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_sctxt;
class cgllvm_modimpl;
class llvm_module;

class cgllvm_general: public cgllvm_sisd{
public:
	typedef cgllvm_sisd parent_class;

	cgllvm_general();

	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( expression_list );
	SASL_VISIT_DCL( index_expression );

	SASL_VISIT_DCL( identifier );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( tynode );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( alias_type );

	// statement

	
private:
	virtual bool create_mod( sasl::syntax_tree::program& v );
	cgllvm_modimpl* mod_ptr();
};

END_NS_SASL_CODE_GENERATOR()

#endif