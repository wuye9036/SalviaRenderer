#ifndef SASL_PARSER_DIAGS_H
#define SASL_PARSER_DIAGS_H

#include <sasl/include/parser/parser_forward.h>
#include <sasl/include/common/diag_item.h>

BEGIN_NS_SASL_PARSER();

extern sasl::common::diag_template cannot_open_input_file;
extern sasl::common::diag_template unrecognized_token;
extern sasl::common::diag_template unknown_tokenize_error;
extern sasl::common::diag_template end_of_file;
extern sasl::common::diag_template unmatched_token;
extern sasl::common::diag_template unmatched_expected_token;

END_NS_SASL_PARSER();

#endif