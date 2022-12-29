#pragma once

#include <eflib/platform/dl_sym_vis.h>

#if defined(salviau_EXPORTS)
#define SALVIA_UTILITY_API EFLIB_SYM_EXPORT
#else
#define SALVIA_UTILITY_API EFLIB_SYM_IMPORT
#endif

namespace salvia::utility {
void SALVIA_UTILITY_API initialize();
void SALVIA_UTILITY_API finalize();

struct scoped_initializer {
  scoped_initializer() { initialize();}
  ~scoped_initializer() { finalize(); }
};

}