#pragma once

#include <eflib/include/platform/dl_sym_vis.h>

#define BEGIN_NS_SALVIAU()	namespace salviau{
#define END_NS_SALVIAU()	}

#if defined(salviau_EXPORTS)
#	define SALVIAU_API EFLIB_SYM_EXPORT
#else
#	define SALVIAU_API EFLIB_SYM_IMPORT
#endif
