#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/diagnostics/assert.h>

#include <vector>

using namespace sasl::syntax_tree;
using namespace llvm;
using namespace sasl::utility;

using std::vector;

BEGIN_NS_SASL_CODE_GENERATOR();

llvm::DefaultIRBuilder* cgllvm_impl::builder() const
{
	return mod->builder().get();
}

llvm::LLVMContext& cgllvm_impl::context() const
{
	return mod->context();
}

llvm::Module* cgllvm_impl::module() const
{
	return mod->module();
}

cgllvm_sctxt* cgllvm_impl::node_ctxt( node* n, bool create_if_need /*= false */ )
{
	shared_ptr<cgllvm_sctxt>& ret = ctxts[n];
	if( ret ){
		return ret.get();
	} else if( create_if_need ){
		ret = create_codegen_context<cgllvm_sctxt>( n->as_handle() );
		return ret.get();
	}

	return NULL;
}

shared_ptr<llvm_module> cgllvm_impl::cg_module() const
{
	return mod;
}

END_NS_SASL_CODE_GENERATOR();