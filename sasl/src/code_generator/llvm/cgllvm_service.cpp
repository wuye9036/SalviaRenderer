#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

using llvm::Module;
using llvm::LLVMContext;
using llvm::DefaultIRBuilder;
using llvm::Value;

BEGIN_NS_SASL_CODE_GENERATOR();

bool cg_service::initialize( llvm_module_impl* mod, node_ctxt_fn const& fn )
{
	assert ( mod );

	mod_impl = mod;
	node_ctxt = fn;

	return true;
}

Module* cg_service::module() const
{
	return mod_impl->module();
}

LLVMContext& cg_service::context() const
{
	return mod_impl->context();
}

DefaultIRBuilder& cg_service::builder() const
{
	return *( mod_impl->builder() );
}

uint32_t value_t::get_masks() const
{
	return masks;
}

void value_t::set_kind( value_kinds vkind )
{
	kind = vkind;
}

value_t* value_t::get_parent() const
{
	return parent.get();
}

END_NS_SASL_CODE_GENERATOR();