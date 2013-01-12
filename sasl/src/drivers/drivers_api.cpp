#include <sasl/include/drivers/drivers_api.h>
#include <sasl/include/drivers/compiler_lib.h>

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

void sasl_create_compiler( boost::shared_ptr<sasl::drivers::compiler>& out )
{
	llvm_initializer::initialize();
	out = sasl::drivers::create_compiler();
}
