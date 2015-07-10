#include <eflib/include/platform/config.h>

#if defined(EFLIB_MSVC)
#   pragma warning(push, 0)
#	pragma warning( disable : 4505 )
#	pragma warning( disable : 4731 )
#	pragma warning( disable : 4702 )
#	pragma warning( disable : 4706 )
#	pragma warning( disable : 4714 )
#endif

#if defined( EFLIB_GCC )
#	pragma GCC push_options
#	pragma GCC diagnostic ignored "-Wall"
#endif
