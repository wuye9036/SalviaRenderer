#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include <fmt/core.h>

#include "Grammar.h"
#include "Meta.h"

namespace pure_static_parsers {
using namespace grammars;

// Parsed data structure and operations
template <typename Tag, typename D>
struct tree_node {
  using tag = Tag;
  using data_type = D;

  std::string_view code_range;
  D data{};

  template <typename Tag2>
  using retagged = tree_node<Tag2, D>;
};

template <typename Tag, typename D>
auto make_tree(std::string_view code_range, D&& data) {
  return tree_node<Tag, std::remove_cvref_t<D>>{.code_range = code_range, .data = ((D &&) data)};
}

template <typename T>
concept is_tree_node = is_specialized<T, tree_node>;

template <typename T>
concept is_optional_tree_node = is_optional<T> && is_tree_node<typename T::value_type>;

template <typename Tag2, typename T>
  requires is_optional_tree_node<std::remove_cvref_t<T>> decltype(auto)
replace_tag(T&& t) {
  using data_type = std::remove_cvref_t<typename T::value_type::data_type>;
  return cast_ref<std::optional<tree_node<Tag2, data_type>>>(((T &&) t));
}

template <typename P>
struct indirect_data;

template <typename T>
  requires is_optional<std::remove_cvref_t<T>>
auto make_or_forward_optional(T&& v) {
  return ((T &&) v);
}

template <typename T>
  requires(!is_optional<std::remove_cvref_t<T>>)
auto make_or_forward_optional(T&& v) {
  return std::make_optional(((T &&) v));
}

template <typename FnT>
auto conditional_(bool cond, FnT&& fn) -> decltype(make_or_forward_optional(((FnT &&) fn)())) {
  if (!cond) {
    return std::nullopt;
  }
  return make_or_forward_optional(((FnT &&) fn)());
}

#define DEBUG_TREE 0
#if DEBUG_TREE == 1
template <typename T>
void print_tree(T&&, std::string indent) {
  fmt::print("{}<unknown_data: {}>", indent, boost::core::demangle(typeid(T).name()));
}
#endif

// return deduced type to resolve 'auto' issue
template <typename P>
struct parse_ {};

template <typename P>
  requires has_combinator<std::remove_cvref_t<P>>
struct parse_<P> {
  using type = std::remove_cvref_t<decltype(replace_tag<P>(
      typename parse_<combinator_t<std::remove_cvref_t<P>>>::type{}))>;
};

template <typename P>
struct parse_<indirect_<P>> {
  using type = std::optional<tree_node<P, std::unique_ptr<indirect_data<P>>>>;
};

template <typename... Ps>
struct parse_<seq<Ps...>> {
  using type =
      std::optional<tree_node<nomad, std::tuple<typename parse_<Ps>::type::value_type...>>>;
};

template <typename... Ps>
struct parse_<br<Ps...>> {
  using type =
      std::optional<tree_node<nomad, std::variant<typename parse_<Ps>::type::value_type...>>>;
};

template <>
struct parse_<lit_int> {
  using type = std::optional<tree_node<lit_int, int>>;
};

template <>
struct parse_<op> {
  using type = std::optional<tree_node<op, char>>;
};

template <typename T>
using parse_t = typename parse_<T>::type;

[[maybe_unused]] void _instantiate_parse_return_types() {
  [[maybe_unused]] auto e = parse_t<expr>{};
}

// #define FUCK_STUPID_COMPILER(deduced_type)
#define FUCK_STUPID_COMPILER(deduced_type) deduced_type

auto parse(std::string_view sv, lit_int) FUCK_STUPID_COMPILER(->parse_t<lit_int>);

template <typename... Ps>
auto parse(std::string_view sv, seq<Ps...>) FUCK_STUPID_COMPILER(->parse_t<seq<Ps...>>);

template <typename... Ps>
auto parse(std::string_view sv, br<Ps...>) FUCK_STUPID_COMPILER(->parse_t<br<Ps...>>);

template <typename P>
auto parse(std::string_view sv, indirect_<P>) FUCK_STUPID_COMPILER(->parse_t<indirect_<P>>);

template <typename OptTree, typename Fn>
  requires is_optional_tree_node<std::remove_cvref_t<OptTree>>
auto operator&(OptTree&& op1, Fn&& op2_factory) {
  using tree_type = typename std::remove_cvref_t<OptTree>::value_type;
  using op1_data_type = typename tree_type::data_type;
  using op2_data_type = std::tuple<typename decltype(((Fn &&) op2_factory)())::value_type>;

  using return_data_type = combine_composible_t<op1_data_type, op2_data_type>;

  return conditional_(op1.has_value(), [&]() {
    auto op2 = ((Fn &&) op2_factory)();
    return conditional_(op2.has_value(), [&] {
      auto fold_code_range = std::string_view{op1->code_range.begin(), op2->code_range.end()};
      return std::make_optional(make_tree<typename tree_type::tag, return_data_type>(
          fold_code_range,
          std::tuple_cat(
              ((OptTree &&) op1).value().data, std::make_tuple(std::move(op2).value()))));
    });
  });
}

template <typename TreeA, typename Fn>
  requires is_optional_tree_node<std::remove_cvref_t<TreeA>>
auto operator|(TreeA&& op1, Fn&& op2_factory) {
  using op1_data_type = typename std::remove_cvref_t<TreeA>::value_type::data_type;
  using op2_data_type = std::variant<typename decltype(((Fn &&) op2_factory)())::value_type>;

  using return_data_type = combine_composible_t<op1_data_type, op2_data_type>;

  auto ret_data = return_data_type{};

  if constexpr (!std::is_same_v<op1_data_type, std::variant<nil>>) {
    if (op1.has_value()) {
      std::visit(
          [&ret_data](auto&& v) { ret_data = std::forward<decltype(v)>(v); }, std::move(op1->data));

      return std::make_optional(make_tree<nomad>(op1->code_range, std::move(ret_data)));
    }
  }

  auto op2 = op2_factory();
  return conditional_(op2.has_value(), [&]() {
    return std::make_optional(make_tree<nomad, return_data_type>(
        op2->code_range, return_data_type{std::move(op2.value())}));
  });
}

auto parse(std::string_view sv, lit_int) FUCK_STUPID_COMPILER(->parse_t<lit_int>) {
  return conditional_(sv.length() >= 1 && ('0' <= sv[0] && sv[0] <= '9'), [&]() {
    auto parsed_range = sv.substr(0, 1);
    auto parsed_data = static_cast<int>(sv[0] - '0');
    return make_tree<lit_int>(parsed_range, parsed_data);
  });
}

auto parse(std::string_view sv, op) FUCK_STUPID_COMPILER(->parse_t<op>) {
  return conditional_(sv.length() >= 1 && (sv[0] == '+'), [&]() {
    auto parsed_range = sv.substr(0, 1);
    auto parsed_data = sv[0];
    return make_tree<op>(parsed_range, parsed_data);
  });
}

template <has_combinator P>
auto parse(std::string_view sv, P) FUCK_STUPID_COMPILER(->parse_t<P>) {
  auto parsed = parse(sv, grammars::combinator_t<P>{});
  return replace_tag<P>(std::move(parsed));
}

template <typename... Ps>
auto parse(std::string_view sv, grammars::seq<Ps...>) FUCK_STUPID_COMPILER(->parse_t<seq<Ps...>>) {
  auto t = std::tuple<Ps...>{};
  return std::apply(
      [&](auto... parsers) {
        auto init_obj = std::make_optional(make_tree<nomad>(sv.substr(0, 0), std::make_tuple()));
        auto parsing_pos = sv.begin();
        return (init_obj & ... & ([&parsing_pos, &sv]() {
                  auto parsed =
                      parse({parsing_pos, sv.end()}, std::remove_cvref_t<decltype(parsers)>{});
                  if (parsed.has_value()) {
                    parsing_pos = parsed->code_range.end();
                  }
                  return parsed;
                }));
      },
      t);
}

template <typename... Ps>
auto parse(std::string_view sv, br<Ps...>) FUCK_STUPID_COMPILER(->parse_t<br<Ps...>>) {
  auto t = std::tuple<Ps...>{};
  return std::apply(
      [&](auto... parsers) {
        auto init_obj = std::make_optional(make_tree<nomad>(sv.substr(0, 0), std::variant<nil>()));
        return (init_obj | ... | ([&]() { return parse(sv, decltype(parsers){}); }));
      },
      t);
}

template <typename P>
struct indirect_data {
  static_assert(has_combinator<P>);

  using parsed_type = parse_t<P>;
  using inner_type = typename parsed_type::value_type::data_type;

  static_assert(!std::is_reference_v<inner_type>);
  inner_type value;
};

template <typename P>
auto parse(std::string_view sv, indirect_<P>) FUCK_STUPID_COMPILER(->parse_t<indirect_<P>>) {
  auto parsed = parse(sv, P{});
  auto data = indirect_data<P>{std::move(parsed.value().data)};
  auto data_ptr = std::make_unique<indirect_data<P>>(std::move(data));

  return conditional_(parsed.has_value(), [&]() {
    return std::make_optional(make_tree<P>(parsed->code_range, std::move(data_ptr)));
  });
}

// Debugging tree
template <typename OptD>
  requires is_optional<std::remove_cvref_t<OptD>>
void print_tree(OptD&& opt, std::string indent) {
  if (!opt) {
    fmt::print("{}{}\n", indent, "<opt: null>");
    return;
  }
  fmt::print("{}{}\n", indent, "optional");
  print_tree(opt.value(), indent + "  ");
}

template <typename Tuple>
  requires is_specialized<std::remove_cvref_t<Tuple>, std::tuple>
void print_tree(Tuple&& t, std::string indent) {
  fmt::print("{}tuple:\n", indent);
  std::apply(
      [&indent](auto&&... args) {
        (print_tree(std::forward<decltype(args)>(args), indent + "  "), ...);
      },
      t);
}

template <typename D>
concept inline_printable_strict =
    (std::same_as<D, std::string_view> || std::integral<D> || std::floating_point<D> ||
     std::same_as<D, std::string>);

template <typename D>
concept inline_printable = inline_printable_strict<std::remove_cvref_t<D>>;

template <typename T>
  requires is_tree_node<std::remove_cvref_t<T>>
void print_tree(T&& t, std::string indent = "") {
  using tree_type = std::decay_t<T>;
  using data_type = typename tree_type::data_type;
  if constexpr (inline_printable<data_type>) {
    fmt::print("{}tree<{}>: {} code: {}\n", indent, tree_type::tag::name, t.data, t.code_range);
  } else {
    fmt::print("{}tree<{}> code: {}\n", indent, tree_type::tag::name, t.code_range);
    print_tree(((T &&) t).data, indent + "  ");
  }
}

template <typename T>
  requires is_specialized<std::remove_cvref_t<T>, std::variant>
void print_tree(T&& t, std::string indent = "") {
  fmt::print("{}variant\n", indent);
  std::visit(
      [&](auto&& v) { print_tree(std::forward<decltype(v)>(v), indent + "  "); }, ((T &&) t));
}

template <typename T>
  requires is_specialized<std::remove_cvref_t<T>, indirect_data>
void print_tree(T&& t, std::string indent = "") {
  fmt::print("{}indirect\n", indent);
  print_tree(((T &&) t).value, indent + "  ");
}

template <typename T>
  requires is_specialized<std::remove_cvref_t<T>, std::unique_ptr>
void print_tree(T&& p, std::string indent = "") {
  fmt::print("{}unique_ptr\n", indent);
  print_tree((*p), indent + "  ");
}
}  // namespace pure_static_parsers

using namespace pure_static_parsers;

TEST(MetaParser, MetaTokenizer) {
  static_assert(std::is_same_v<combinator_t<expr_mono>, seq<lit_int, op, lit_int>>);

  auto r1 = pure_static_parsers::parse("1+2", grammars::expr_mono{});
  print_tree(r1, "");

  auto r2 = pure_static_parsers::parse("1+2+3", grammars::expr{});

  print_tree(r2, "");
}
