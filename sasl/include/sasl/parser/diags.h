#pragma once

#include <sasl/common/diag_item.h>

namespace sasl::parser{

// Boost.Wave Errors
extern sasl::common::diag_template boost_wave_exception_warning;
extern sasl::common::diag_template boost_wave_exception_error;
extern sasl::common::diag_template boost_wave_exception_fatal_error;

extern sasl::common::diag_template cannot_open_include_file;
extern sasl::common::diag_template cannot_open_input_file;
extern sasl::common::diag_template unrecognized_token;
extern sasl::common::diag_template unknown_tokenize_error;
extern sasl::common::diag_template end_of_file;
extern sasl::common::diag_template unmatched_token;
extern sasl::common::diag_template unmatched_expected_token;

}
