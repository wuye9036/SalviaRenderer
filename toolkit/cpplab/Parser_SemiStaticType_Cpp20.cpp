#include <gtest/gtest.h>

#include <memory>
#include <type_traits>

#include <fmt/core.h>
#include <nameof.hpp>

#include "Grammar.h"
#include "Meta.h"

using namespace grammars;

template <typename Visitor>
class attribute {
public:
  template <typename Visitor2>
  void accept(Visitor2& visitor) {
    attr_impl_->accept(visitor);
  }

  [[nodiscard]] std::string_view token_range() const noexcept {
    if (empty()) {
      return {};
    }
    return attr_impl_->token_range;
  }

  struct _concept {
    std::string_view token_range{};

    virtual void accept(Visitor& visitor) = 0;

    virtual ~_concept() = default;
  };

  template <typename Rule, typename AttrValue>
  struct _model final : _concept {
    template <typename AttrValue2>
    explicit _model(AttrValue2&& value, std::string_view token_range)
      : attr_value{(AttrValue2 &&) value} {
      this->token_range = token_range;
    }

    void accept(Visitor& visitor) override { visitor(Rule{}, attr_value); }

    AttrValue attr_value;
  };

  template <typename Rule, typename AttrValue>
  attribute(Rule, AttrValue&& value, std::string_view token_range)
    : attr_impl_{std::make_unique<_model<Rule, std::remove_cvref_t<AttrValue>>>(
          (AttrValue &&) value, token_range)} {}

  template <typename Rule>
  explicit attribute(Rule) : attr_impl_{} {}

  [[nodiscard]] bool empty() const noexcept { return attr_impl_ == nullptr; }

private:
  std::unique_ptr<_concept> attr_impl_;
};

template <typename... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
overloads(Ts&&...) -> overloads<Ts...>;

template <typename Visitor>
struct parse_ {
  using attribute = attribute<Visitor>;

  template <typename RuleAlias>
  attribute operator()(std::string_view sv, lit_int r, RuleAlias) const {
    if (sv.length() >= 1 && ('0' <= sv[0] && sv[0] <= '9')) {
      auto parsed_range = sv.substr(0, 1);
      auto parsed_data = static_cast<int>(sv[0] - '0');
      return {r, parsed_data, parsed_range};
    }
    return attribute{r};
  }

  template <typename RuleAlias>
  attribute operator()(std::string_view sv, op r, RuleAlias) const {
    if (sv.length() >= 1 && sv[0] == '+') {
      auto parsed_range = sv.substr(0, 1);
      auto parsed_data = sv[0];
      return {r, parsed_data, parsed_range};
    }
    return attribute{r};
  }

  template <typename Iterator, typename Aggregate, typename... Rs>
  attribute parse_combination(Iterator begin, Iterator end, Aggregate&& aggregate, Rs...) const {
    auto cursor = begin;

    std::vector<std::function<attribute()>> children_parse{[&cursor, end, this]() {
      auto ret = (*this)({cursor, end}, Rs{}, Rs{});
      return ret;
    }...};

    return std::invoke((Aggregate &&) aggregate, children_parse, cursor);
  }

  template <typename... Ps, typename RuleAlias>
  attribute operator()(std::string_view sv, seq<Ps...>, RuleAlias a) const {
    return parse_combination(
        sv.begin(),
        sv.end(),
        [sv, a](auto&& children_parse, auto& cursor) {
          std::vector<attribute> ret;
          for (auto&& parse_child : children_parse) {
            auto attr = parse_child();
            // return if fail
            if (attr.empty()) {
              return attribute{a};
            }
            // update cursor
            cursor = attr.token_range().end();
            ret.push_back(std::move(attr));
          }

          return attribute{a, std::move(ret), {sv.begin(), cursor}};
        },
        Ps{}...);
  }

  template <typename... Ps, typename RuleAlias>
  attribute operator()(std::string_view sv, br<Ps...>, RuleAlias a) const {
    return parse_combination(
        sv.begin(),
        sv.end(),
        [a](auto&& children_parse, auto& cursor) -> attribute {
          std::vector<attribute> ret;
          for (size_t i = 0; i < children_parse.size(); ++i) {
            auto attr = children_parse[i]();
            // return if success
            if (!attr.empty()) {
              auto token_range = attr.token_range();
              return {a, std::make_pair(i, std::move(attr)), token_range};
            }
          }

          return attribute{a};
        },
        Ps{}...);
  }

  template <typename P, typename RuleAlias>
  attribute operator()(std::string_view sv, indirect_<P>, RuleAlias) const {
    return (*this)(sv, P{}, P{});
  }

  template <has_combinator P, typename RuleAlias>
  attribute operator()(std::string_view sv, P, RuleAlias a) const {
    return (*this)(sv, combinator_t<P>{}, a);
  }

  template <typename P>
  attribute operator()(std::string_view sv, P) const {
    return (*this)(sv, P{}, P{});
  }
};

template <typename Visitor>
inline constexpr auto parse = parse_<Visitor>{};

struct Visitor {
  std::string indent{};

  template <typename R, typename Attr>
  void operator()(R, Attr&& attr) {
    fmt::print(
        "{}{}: <{}>.\n", indent, nameof::nameof_short_type<R>(), nameof::nameof_type<decltype(attr)>());
  }

  template <typename R, typename SubAttr>
  void operator()(R r, std::vector<SubAttr>& attr) {
    fmt::print(
        "{}{}: <{}>.\n",
        indent,
        nameof::nameof_short_type<R>(),
        nameof::nameof_type<decltype(attr)>());
    auto old_indent = indent;
    indent += "  ";
    for (auto& sub_attr : attr) {
      sub_attr.accept(*this);
    }
    indent = old_indent;
  }

  template <typename R, std::integral IndexT, typename Attr>
  void operator()(R r, std::pair<IndexT, Attr>& attr) {
    fmt::print(
        "{}{}: <{}>.\n",
        indent,
        nameof::nameof_short_type<R>(),
        nameof::nameof_type<decltype(attr)>());
    auto old_indent = indent;
    indent += "  ";
    attr.second.accept(*this);
    indent = old_indent;
  }
};

TEST(SemiStaticAttribute, SemiStaticAttribute) {
  auto visitor = Visitor{};

  auto attr1 = parse<Visitor>("1+2", grammars::expr_mono{});

  auto attr2 = parse<Visitor>("1+2+3", grammars::expr{});

  attr1.accept(visitor);
  attr2.accept(visitor);
}
