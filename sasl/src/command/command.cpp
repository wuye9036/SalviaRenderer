#include <eflib/platform/config.h>
#include <eflib/diagnostics/assert.h>
#include <eflib/platform/dl_loader.h>
#include <sasl/common/diag_chat.h>
#include <sasl/common/diag_formatter.h>
#include <sasl/drivers/compiler.h>

using eflib::dynamic_lib;
using sasl::common::diag_chat;
using sasl::common::diag_item;
using sasl::common::report_handler_fn;
using sasl::common::str;
using sasl::drivers::compiler;
using std::cout;
using std::endl;
using std::shared_ptr;

#if defined(EFLIB_WINDOWS)
#  define DRIVER_EXT ".dll"
#elif defined(EFLIB_LINUX) || defined(EFLIB_MACOS)
#  define DRIVER_EXT ".so"
#endif

#ifdef EFLIB_DEBUG
#  define DRIVER_NAME "sasl_drivers_d"
#else
#  define DRIVER_NAME "sasl_drivers"
#endif

bool on_diag_item_reported(diag_chat*, diag_item* item) {
  cout << str(item) << endl;
  return true;
}

int main(int argc, char** argv) {
  shared_ptr<diag_chat> diags = diag_chat::create();
  diags->add_report_raised_handler(report_handler_fn(on_diag_item_reported));

  void (*pfn)(shared_ptr<compiler>&) = nullptr;
  shared_ptr<dynamic_lib> driver_lib =
      dynamic_lib::load(std::string(DRIVER_NAME) + std::string(DRIVER_EXT));
  driver_lib->get_function(pfn, "sasl_create_compiler");
  shared_ptr<compiler> drv;
  pfn(drv);
  drv->set_parameter(argc, argv);
  shared_ptr<diag_chat> comp_diags = drv->compile(false);

  diag_chat::merge(diags.get(), comp_diags.get(), true);

#if defined(EFLIB_DEBUG)
  system("pause");
#endif

  return 0;
}
