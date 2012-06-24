#include <sasl/include/driver/driver_api.h>
#include <sasl/include/driver/driver_lib.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>
#include <eflib/include/platform/enable_warnings.h>

void sasl_create_driver( boost::shared_ptr<sasl::driver::driver>& out )
{
	out = sasl::driver::create_driver();
}

void sasl_initialize_driver()
{
	llvm::InitializeNativeTarget();
}

void sasl_finalize_driver()
{
	llvm::llvm_shutdown();
}
