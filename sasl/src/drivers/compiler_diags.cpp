#include <sasl/include/drivers/compiler_diags.h>

using namespace sasl::common;

BEGIN_NS_SASL_DRIVERS();

diag_template text_only( dl_text, "%s" );
diag_template unknown_detail_level( dl_text, "Detail Level '%s' is invalid and was ignored." );
diag_template input_file_is_missing( dl_text, "Input File is missing. Please specify input file at least one." );
diag_template unknown_lang( dl_text, "Language of input file(s) is unknown. Specify it by --lang=<language name>." );
diag_template compiling_input( dl_text, "Compiling '%s' ..." );

END_NS_SASL_DRIVERS();