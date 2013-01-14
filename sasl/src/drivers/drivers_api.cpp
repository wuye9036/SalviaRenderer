#include <sasl/include/drivers/drivers_api.h>
#include <sasl/include/drivers/compiler_lib.h>
#include <sasl/include/shims/ia_shim.h>
#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/memory/atomic.h>

class llvm_initializer
{
public:
	llvm_initializer()
	{
		llvm::InitializeNativeTarget();
	}

	~llvm_initializer()
	{
		llvm::llvm_shutdown();
	}

	static llvm_initializer& initialize()
	{
		static llvm_initializer obj;
		return obj;
	}
};

void sasl_create_compiler(sasl::drivers::compiler_ptr& out )
{
	llvm_initializer::initialize();
	out = sasl::drivers::create_compiler();
}

void sasl_create_ia_shim(sasl::shims::ia_shim_ptr& out)
{
	llvm_initializer::initialize();
	out = sasl::shims::ia_shim::create();
}