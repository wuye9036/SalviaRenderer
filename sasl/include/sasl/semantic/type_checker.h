#pragma once

#include <sasl/semantic/semantic_forward.h>

#include <memory>
#include <string>
namespace sasl {
namespace syntax_tree {
struct tynode;
struct function_full_def;
struct builtin_type;
} // namespace syntax_tree
} // namespace sasl

namespace sasl::semantic {

bool type_equal(std::shared_ptr<::sasl::syntax_tree::tynode> lhs,
                std::shared_ptr<::sasl::syntax_tree::tynode> rhs);

bool type_equal(std::shared_ptr<::sasl::syntax_tree::builtin_type> lhs,
                std::shared_ptr<::sasl::syntax_tree::builtin_type> rhs);

// std::shared_ptr<::sasl::syntax_tree::tynode> actual_type(
// std::shared_ptr<::sasl::syntax_tree::tynode> );
} // namespace sasl::semantic
