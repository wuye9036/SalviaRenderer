#include <eflib/include/platform/config.h>
#include <eflib/include/platform/dl_loader.h>

using sasl::driver;
using eflib::dynamic_lib;

int main (int argc, char **argv){

	shared_ptr<dynamic_lib> eflib::dynamic_lib;

	bool aborted = false;

	compiler c;

	c.parse(argc, argv);
	c.process( aborted );

#if defined(EFLIB_DEBUG)
	system("pause");
#endif

	return 0;
}