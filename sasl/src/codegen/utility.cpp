#include <sasl/include/codegen/utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/IR/Function.h>
#include <eflib/include/platform/enable_warnings.h>

using llvm::Function;

namespace sasl::codegen {

void dbg_print_blocks( Function* fn )
{
#ifdef _DEBUG
	/*printf( "Function: 0x%X\n", fn );
	for( Function::BasicBlockListType::iterator it = fn->getBasicBlockList().begin(); it != fn->getBasicBlockList().end(); ++it ){
	printf( "  Block: 0x%X\n", &(*it) );
	}*/
	fn = fn;
#else
	fn = fn;
#endif
}

}
