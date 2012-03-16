#include <eflib/include/platform/config.h>

#include <sasl/include/compiler/options.h>

using sasl::compiler::compiler;
using sasl::compiler::options_io;

int main (int argc, char **argv){

	bool aborted = false;

	compiler c;

	c.parse(argc, argv);
	c.process( aborted );

#if defined(EFLIB_DEBUG)
	system("pause");
#endif

	return 0;
}