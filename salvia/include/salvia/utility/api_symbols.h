#pragma once

#include <eflib/platform/dl_sym_vis.h>

#if defined(salviau_EXPORTS)
#define SALVIA_UTILITY_API EFLIB_SYM_EXPORT
#else
#define SALVIA_UTILITY_API EFLIB_SYM_IMPORT
#endif
