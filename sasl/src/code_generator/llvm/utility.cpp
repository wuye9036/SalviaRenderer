#include <sasl/include/code_generator/llvm/utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Function.h>
#include <eflib/include/platform/enable_warnings.h>

using llvm::Function;

BEGIN_NS_SASL_CODE_GENERATOR();

void mask_to_indexes( char indexes[4], uint32_t mask )
{
	for( int i = 0; i < 4; ++i ){
		// XYZW is 1,2,3,4 but LLVM used 0,1,2,3
		char comp_index = static_cast<char>( (mask >> i*8) & 0xFF );
		if( comp_index == 0 ){
			indexes[i] = -1;
			break;
		}
		indexes[i] = comp_index - 1;
	}
}

uint32_t indexes_to_mask( char indexes[4] )
{
	uint32_t mask = 0;
	for( int i = 0; i < 4; ++i ){
		mask += (uint32_t)( (indexes[i] + 1) << (i*8) );
	}
	return mask;
}

uint32_t indexes_to_mask( char idx0, char idx1, char idx2, char idx3 )
{
	char indexes[4] = { idx0, idx1, idx2, idx3 };
	return indexes_to_mask( indexes );
}

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

uint32_t indexes_length( char indexes[4] )
{
	int i = 0;
	while( indexes[i] != -1 && i < 4 ){ ++i; }
	return i;
}

uint32_t indexes_length( uint32_t mask )
{
	char indexes[4] = {-1, -1, -1, -1};
	mask_to_indexes(indexes, mask);
	return indexes_length(indexes);
}

END_NS_SASL_CODE_GENERATOR();