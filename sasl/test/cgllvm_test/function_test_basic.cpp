#include <sasl/include/code_generator/llvm/cgllvm_api.h>

#include <eflib/include/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <eflib/include/enable_warnings.h>

#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/make_tree.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/semantic/semantic_analyser.h>

#include <boost/test/unit_test.hpp>

using sasl::code_generator::generate_llvm_code;
using sasl::code_generator::llvm_code;

using sasl::common::compiler_info_manager;

using sasl::semantic::semantic_analysis;

using sasl::syntax_tree::node;
using sasl::syntax_tree::program;

using sasl::syntax_tree::dprog_combinator;


BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( function_generation_test ){
	boost::shared_ptr<program> prog;
	boost::shared_ptr<compiler_info_manager> cim = compiler_info_manager::create();

	dprog_combinator( "test" )
		.dfunction( "foo" )
		.dreturntype().dbuildin( buildin_type_code::_void ).end()
		.end()
	.end( prog );

	semantic_analysis( prog, cim );
	boost::shared_ptr<llvm_code> retcode = generate_llvm_code( prog );
	llvm::Function* func = retcode->module()->getFunction( "foo" );
	BOOST_CHECK( func );
	BOOST_CHECK( func->getFunctionType()->getReturnType() );
	BOOST_CHECK( func->getFunctionType()->getReturnType()->isVoidTy() );
}

BOOST_AUTO_TEST_SUITE_END()
