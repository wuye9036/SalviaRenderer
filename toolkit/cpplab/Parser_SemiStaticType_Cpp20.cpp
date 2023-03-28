#include <gtest/gtest.h>

#include <memory>
#include <type_traits>

#include "Grammar.h"

using namespace grammars;

template <typename Visitor>
class attribute {
public:
  template <typename Visitor2>
  void accept(Visitor2& visitor) {
    attr_impl_->accept(visitor);
  }

  template <typename Visitor2>
  void accept(Visitor2 const& visitor) {
    attr_impl_->accept(visitor);
  }

  struct _concept {
    virtual void accept(Visitor& visitor) = 0;

    virtual void accept(Visitor const& visitor) = 0;

    virtual ~_concept() = default;
  };

  template <typename Rule, typename AttrValue>
  struct _model final : _concept {
    explicit _model(AttrValue value) : attr_value{std::move(value)} {}

    void accept(Visitor& visitor) override { visitor(Rule{}, attr_value); }

    void accept(Visitor const& visitor) override { visitor(Rule{}, attr_value); }

    AttrValue attr_value;
  };

  template <typename Rule, typename AttrValue>
  attribute(Rule, AttrValue&& value)
    : attr_impl_{std::make_unique<_model<Rule, AttrValue>>((AttrValue &&) value)} {}

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

  attribute operator()(std::string_view sv, lit_int r) const { return {r, 0}; }

  attribute operator()(std::string_view sv, op r) const { return {r, '+'}; }

  template <typename... Ps, typename Alias>
  attribute operator()(std::string_view sv, seq<Ps...> r, Alias a) const {
    std::vector<attribute> ret;
    return {a, std::move(ret)};
  }

  template <typename... Ps, typename Alias>
  attribute operator()(std::string_view sv, br<Ps...> r, Alias a) const {
    return {a, std::make_pair(-1, attribute{a})};
  }

  template <typename P>
  attribute operator()(std::string_view sv, indirect_<P> r) const {
    return (*this)(sv, P{});
  }

  template <has_combinator P>
  attribute operator()(std::string_view sv, P r) const {
    return (*this)(sv, combinator_t<P>{}, r);
  }
};

template <typename Visitor>
inline constexpr auto parse = parse_<Visitor>{};

TEST(SemiStaticAttribute, SemiStaticAttribute) {
  auto visitor = overloads{[](auto r, auto&& v) {
  }};

  auto attr1 = parse<decltype(visitor)>("1+2", grammars::expr_mono{});

  auto attr2 = parse<decltype(visitor)>("1+2+3", grammars::expr{});

  attr1.accept(visitor);
  attr2.accept(visitor);
}
