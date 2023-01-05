#pragma once

#include <sasl/common/diag_item.h>

namespace sasl::drivers {
using sasl::common::diag_levels;
using sasl::common::diag_template;

constexpr diag_template text_only{3000, diag_levels::debug, "{}"};
constexpr diag_template unknown_detail_level{3001, diag_levels::info,
                                             "Detail Level '{}' is invalid and was ignored."};
constexpr diag_template input_file_is_missing{
    3002, diag_levels::fatal_error, "Input File is missing. Please specify input file at least one."};
constexpr diag_template unknown_lang{
    3003, diag_levels::error,
    "Language of input file(s) is unknown. Specify it by --lang=<language name>."};
constexpr diag_template compiling_input{3004, diag_levels::info, "Compiling '{}' ..."};
} // namespace sasl::drivers
