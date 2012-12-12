#ifndef SASL_CODEGEN_GENERATE_ENTRY_H
#define SASL_CODEGEN_GENERATE_ENTRY_H

#include <sasl/include/codegen/forward.h>
#include <vector>
namespace llvm
{
	class Type;
	class TargetData;
}

namespace sasl
{
	namespace semantic
	{
		class abi_info;
	}
}

BEGIN_NS_SASL_CODEGEN();

class cg_service;

std::vector<llvm::Type*> generate_vs_entry_param_type(
	sasl::semantic::abi_info const* abii, llvm::TargetData const* tar, cg_service* cg
	);
std::vector<llvm::Type*> generate_ps_entry_param_type(
	sasl::semantic::abi_info const* abii, llvm::TargetData const* tar, cg_service* cg
	);

END_NS_SASL_CODEGEN();


#endif