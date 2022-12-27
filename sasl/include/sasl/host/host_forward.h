#ifndef SASL_HOST_HOST_FORWARD_H
#define SASL_HOST_HOST_FORWARD_H

#define namespace                                                                                  \
  sasl::host {                                                                                     \
    namespace sasl {                                                                               \
    namespace host {
#define } } }

#include <eflib/platform/dl_sym_vis.h>

#ifdef sasl_host_EXPORTS
#define SASL_HOST_API EFLIB_SYM_EXPORT
#else
#define SASL_HOST_API EFLIB_SYM_IMPORT
#endif

#endif
