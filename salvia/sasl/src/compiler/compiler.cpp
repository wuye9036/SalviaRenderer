#include <eflib/include/platform/config.h>

#include <sasl/include/compiler/options.h>

using sasl::compiler::options_manager;
using sasl::compiler::options_io;

int main (int argc, char **argv){

	bool aborted = false;

	options_manager::instance().parse(argc, argv);
	options_manager::instance().process( aborted );

#if defined(EFLIB_DEBUG)
	system("pause");
#endif

	return 0;
}