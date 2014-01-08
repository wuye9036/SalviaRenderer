#ifndef SASL_HOST_HOST_FORWARD_H
#define SASL_HOST_HOST_FORWARD_H

#define BEGIN_NS_SASL_HOST() namespace sasl{ namespace host{
#define END_NS_SASL_HOST() }}

#include <eflib/include/platform/dl_sym_vis.h>

#ifdef sasl_host_EXPORTS
#define SASL_HOST_API EFLIB_SYM_EXPORT
#else
#define SASL_HOST_API EFLIB_SYM_IMPORT
#endif

#endif
