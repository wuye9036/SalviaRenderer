#pragma once

#include <sasl/common/diag_item.h>
#include <string_view>

namespace sasl::parser::diags {

using sasl::common::diag_levels;
using sasl::common::diag_template;
// clang-format off

// Boost.Wave errors
constexpr sasl::common::diag_template boost_wave_exception_warning        {1000, diag_levels::warning,        "token: {token:s}"};
constexpr sasl::common::diag_template boost_wave_exception_error          {1001, diag_levels::error,          "token: {token:s}"};
constexpr sasl::common::diag_template boost_wave_exception_fatal_error	  {1002, diag_levels::fatal_error,    "token: {token:s}"};

// SASL errors
constexpr sasl::common::diag_template cannot_open_include_file            {1100, diag_levels::error,          "cannot open include file: '{file_name:s}': no such file or directory"};
constexpr sasl::common::diag_template cannot_open_input_file              {1101, diag_levels::error,          "cannot open input file: '{file_name:s}'."};
constexpr sasl::common::diag_template unrecognized_token                  {1102, diag_levels::error,          "unrecognized token: '{token:s}'."};
constexpr sasl::common::diag_template unknown_tokenize_error              {1103, diag_levels::error,          "unknown exception in tokenize stage was raised. exception info: '{info:s}'."};
constexpr sasl::common::diag_template end_of_file                         {1104, diag_levels::error,          "end of file found but '{:s}' needed."};
constexpr sasl::common::diag_template unmatched_token                     {1105, diag_levels::error,          "syntax error: '{syntax_error:s}'"};
constexpr sasl::common::diag_template unmatched_expected_token            {1106, diag_levels::error,          "syntax error: missing '{:s}' before '{:s}'."};

// clang-format on
}  // namespace sasl::parser::diags
