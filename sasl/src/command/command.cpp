#include <eflib/include/platform/config.h>
#include <sasl/include/drivers/compiler.h>
#include <sasl/include/common/diag_chat.h>
#include <sasl/include/common/diag_formatter.h>
#include <eflib/include/platform/dl_loader.h>
#include <eflib/include/diagnostics/assert.h>

using sasl::drivers::compiler;
using sasl::common::diag_chat;
using sasl::common::diag_item;
using sasl::common::str;
using sasl::common::report_handler_fn;
using eflib::dynamic_lib;
using boost::shared_ptr;
using std::cout;
using std::endl;

#ifdef EFLIB_WINDOWS
#	define DRIVER_EXT ".dll"
#else
#	error "Unknown OS."
#endif

#ifdef EFLIB_DEBUG
#	define DRIVER_NAME "sasl_drivers_d"
#else
#	define DRIVER_NAME "sasl_drivers"
#endif

bool on_diag_item_reported( diag_chat*, diag_item* item )
{
	cout << str(item) << endl;
	return true;
}

int main (int argc, char **argv){

	shared_ptr<diag_chat> diags = diag_chat::create();
	diags->add_report_raised_handler( report_handler_fn(on_diag_item_reported) );

	void (*pfn)( shared_ptr<compiler>& ) = NULL;
	shared_ptr<dynamic_lib> driver_lib = dynamic_lib::load( std::string(DRIVER_NAME) + std::string(DRIVER_EXT) );
	driver_lib->get_function( pfn, "sasl_create_compiler" );
	shared_ptr<compiler> drv;
	pfn(drv);
	drv->set_parameter( argc, argv );
	shared_ptr<diag_chat> comp_diags = drv->compile(true);

	diag_chat::merge(diags.get(), comp_diags.get(), true);

#if defined(EFLIB_DEBUG)
	system("pause");
#endif

	return 0;
}