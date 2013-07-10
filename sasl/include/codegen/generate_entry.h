#ifndef SASL_CODEGEN_GENERATE_ENTRY_H
#define SASL_CODEGEN_GENERATE_ENTRY_H

#include <sasl/include/codegen/forward.h>
#include <vector>
namespace llvm
{
	class Type;
	class DataLayout;
}

namespace sasl
{
	namespace semantic
	{
		class reflection_impl;
	}
}

BEGIN_NS_SASL_CODEGEN();

class cg_service;

std::vector<llvm::Type*> generate_vs_entry_param_type(
	sasl::semantic::reflection_impl const* abii, llvm::DataLayout const* dataLayout, cg_service* cg
	);
std::vector<llvm::Type*> generate_ps_entry_param_type(
	sasl::semantic::reflection_impl const* abii, llvm::DataLayout const* dataLayout, cg_service* cg
	);

END_NS_SASL_CODEGEN();


#endif