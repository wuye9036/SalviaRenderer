#pragma once

#include <sasl/common/token.h>

#include <eflib/utility/composition.h>

#include <optional>
#include <variant>

namespace sasl::parser_next {

using token = sasl::common::token;
using namespace eflib::composition;

struct attribute;

struct attribute_base {
  intptr_t rule_id;
  token token_beg, token_end;
};

struct terminal_attribute : attribute_base {};

// *|+|- rule
struct select_attribute : attribute_base {
  std::optional<indirect_<attribute>> attr;
  size_t selected_index;
};

// rule0 | rule1
struct sequence_attribute : attribute_base {
  std::vector<attribute> children;
};

// rule0 >> rule1
// rule0 > rule1
struct queue_attribute : attribute_base {
  std::vector<attribute> children;
};

struct attribute
    : std::variant<terminal_attribute, select_attribute, sequence_attribute, queue_attribute> {};

} // namespace sasl::parser_next