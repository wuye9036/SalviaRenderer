#include <salviau/include/common/presenter_utility.h>

#include <eflib/include/platform/dl_loader.h>

#include <eflib/include/memory/atomic.h>

#if defined( SALVIA_BUILD_WITH_DIRECTX )
#	define PRESENTER_NAME "d3d9"
#else
#	define PRESENTER_NAME "opengl"
#endif

#if defined(EFLIB_DEBUG)
#	define DEBUG_POSTFIX "_d"
#else
#	define DEBUG_POSTFIX ""
#endif

EFLIB_USING_SHARED_PTR(salviar, device);
using eflib::dynamic_lib;
using std::string;

BEGIN_NS_SALVIAU();

static boost::shared_ptr<dynamic_lib> presenter_lib;
static eflib::spinlock mutex;

typedef void (*create_present_func_ptr)(salviar::device_ptr& dev, void* param);

salviar::device_ptr create_default_presenter(void* param)
{
	eflib::scoped_spin_locker locker(mutex);
	
	device_ptr ret;

	if(!presenter_lib)
	{
		std::string library_name = "salviax_";
		library_name += PRESENTER_NAME;
		library_name += "_presenter";
		library_name += DEBUG_POSTFIX;
		library_name += ".dll";
		
		presenter_lib = dynamic_lib::load(library_name);
	}
	
	if(!presenter_lib)
	{
		return ret;
	}

	create_present_func_ptr create_presenter = NULL;
	presenter_lib->get_function(create_presenter, "salviax_create_presenter_device");
	
	if(create_presenter)
	{
		create_presenter(ret, param);
	}

	return ret;
}

END_NS_SALVIAU();
