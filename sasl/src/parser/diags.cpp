#include <sasl/include/parser/diags.h>

using namespace sasl::common;

BEGIN_NS_SASL_PARSER();

diag_template cannot_open_input_file( dl_error, "Cannot open input file: '%s'." );
diag_template unrecognized_token( dl_error, "Unrecognized token: '%s'." );
diag_template unknown_tokenize_error( dl_error, "Unknown exception in tokenize stage was raised. exception info: '%s'." );

END_NS_SASL_PARSER();