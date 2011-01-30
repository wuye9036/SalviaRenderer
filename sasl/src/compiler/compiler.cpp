#include <sasl/include/compiler/options.h>

int main (int argc, char **argv){

	bool aborted = false;

	sasl::compiler::options_manager::instance().parse(argc, argv);
	sasl::compiler::options_manager::instance().process( aborted );

	return 0;

}