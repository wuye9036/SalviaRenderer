#include <sasl/host/shader_object_impl.h>

#include <sasl/semantic/reflection_impl.h>
#include <sasl/codegen/cg_api.h>

using namespace salviar;
using namespace sasl::semantic;
using namespace sasl::codegen;

namespace sasl::host() {

shader_object_impl::shader_object_impl()
	:entry_(nullptr)
{
}

shader_reflection const* shader_object_impl::get_reflection() const
{
	return reflection_.get();
}

void* shader_object_impl::native_function() const
{
	if(!entry_)
	{
		const_cast<shader_object_impl*>(this)->entry_
			= module_vmc_->get_function( reflection_->entry_name() );
	}
	return entry_;
}

void shader_object_impl::set_reflection(shader_reflection_ptr const& reflection)
{
	reflection_ = std::static_pointer_cast<reflection_impl>(reflection);
}

void shader_object_impl::set_module_semantic(module_semantic_ptr const& semantic)
{
	module_sem_ = semantic;
}

void shader_object_impl::set_module_context(module_context_ptr const& context)
{
	module_ctx_ = context;
}

void shader_object_impl::set_vm_code(module_vmcode_ptr const& vmcode)
{
	module_vmc_ = vmcode;
}

}