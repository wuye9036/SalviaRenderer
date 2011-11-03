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

#define SASL_VISITOR_TYPE_NAME cgllvm_impl

cgllvm_impl::cgllvm_impl(): abii(NULL), msi(NULL){
}

SASL_VISIT_DEF_UNIMPL( declaration );

llvm::DefaultIRBuilder* cgllvm_impl::builder() const{
	return mod->builder().get();
}

boost::shared_ptr<llvm_module> cgllvm_impl::cg_module() const{
	return mod;
}

llvm::LLVMContext& cgllvm_impl::context() const{
	return mod->context();
}

llvm::Module* cgllvm_impl::module() const{
	return mod->module();
}

cgllvm_sctxt* cgllvm_impl::node_ctxt( shared_ptr<node> const& n, bool create_if_need /*= false */ )
{
	return node_ctxt<node, cgllvm_sctxt>( n, create_if_need );
}

END_NS_SASL_CODE_GENERATOR();