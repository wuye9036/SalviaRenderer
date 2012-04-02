#include <sasl/include/parser/diags.h>

using namespace sasl::common;

BEGIN_NS_SASL_PARSER();

// Boost.Wave
diag_template boost_wave_exception_warning		( dl_warning,		"token: %s");
diag_template boost_wave_exception_error		( dl_error,			"token: %s");
diag_template boost_wave_exception_fatal_error	( dl_fatal_error,	"token: %s");

diag_template cannot_open_include_file( dl_error, "cannot open include file: '%s': no such file or directory" );
diag_template cannot_open_input_file( dl_error, "cannot open input file: '%s'." );
diag_template unrecognized_token( dl_error, "unrecognized token: '%s'." );
diag_template unknown_tokenize_error( dl_error, "unknown exception in tokenize stage was raised. exception info: '%s'." );
diag_template end_of_file( dl_error, "end of file found but '%s' needed." );
diag_template unmatched_token( dl_error, "syntax error: '%s'" );
diag_template unmatched_expected_token( dl_error, "syntax error: missing '%s' before '%s'." ); 

END_NS_SASL_PARSER();