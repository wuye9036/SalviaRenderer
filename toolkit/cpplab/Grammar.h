#pragma once

#include <string_view>
#include <type_traits>

namespace grammars {
// Combinators
template <typename... Ps>
struct seq {};
template <typename P>
struct rep {};
template <typename... Ps>
struct br {};
template <typename P>
struct indirect_ {};

// Rule declarations
struct nomad {
  static constexpr auto name = "nomad";
};
struct expr_mono {
  static constexpr auto name = "expr_mono";
};
struct expr {
  static constexpr auto name = "expr";
};
struct lit_int {
  static constexpr auto name = "lit_int";
};
struct op {
  static constexpr auto name = "op";
};

// Rule definitions and concepts
template <typename T>
struct combinator {};

template <>
struct combinator<expr_mono> {
  using type = seq<lit_int, op, lit_int>;
};

template <>
struct combinator<expr> {
  using type = br<seq<lit_int, op, indirect_<expr>>, lit_int>;
};

template <typename T>
using combinator_t = typename combinator<std::remove_cvref_t<T>>::type;

template <typename T>
concept has_combinator = requires { combinator_t<T>{}; };
}  // namespace grammars