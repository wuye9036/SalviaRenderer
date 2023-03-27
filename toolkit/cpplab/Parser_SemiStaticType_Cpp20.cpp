#include <gtest/gtest.h>

#include <memory>
#include <type_traits>

class attribute {
public:
  template <typename T>
  attribute& operator=(T&) {}

  template <typename T>
  attribute& operator=(T&&) {}

  void accept() { attr_impl_->accept(); }

  struct _concept {
    virtual void accept() = 0;
    virtual ~_concept() = default;
  };

  template <typename Visitor, typename Rule, typename AttrValue>
  struct _model final : _concept {
    template <typename AttrValue2>
    _model(Visitor& visitor, AttrValue2&& value)
      : visitor(visitor)
      , attr_value{((AttrValue2 &&) value)} {}
    void accept() override { visitor(Rule{}, attr_value); }
    Visitor& visitor;
    AttrValue attr_value;
  };

  template <typename Visitor, typename Rule, typename AttrValue>
  attribute(Visitor& visitor, Rule, AttrValue&& value)
    : attr_impl_{
          std::make_unique<_model<Visitor, Rule, AttrValue>>(visitor, (AttrValue &&) value)} {}

private:
  std::unique_ptr<_concept> attr_impl_;
};

class value_of_seq {
  std::vector<attribute> children;
};

class value_of_queue {
  std::vector<attribute> children;
};

class value_of_select {
  std::unique_ptr<attribute> selected;
};

class value_of_term {};

struct rules {
  struct expr {};
};

template <typename... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
overloads(Ts&&...) -> overloads<Ts...>;

template <typename Visitor, typename Grammar, typename TokenRange>
attribute parse(Visitor& visitor, Grammar const& g, TokenRange tokens) {
  return {visitor, rules::expr{}, value_of_term{}};
}

struct grammar {};

struct token_range {};

TEST(SemiStaticAttribute, SemiStaticAttribute) {
  auto visitor = overloads{[](auto r, auto&& v) {
    static_assert(std::is_same_v<rules::expr, std::remove_cvref_t<decltype(r)>>);
  }};

  auto attr = parse(visitor, grammar{}, token_range{});

  attr.accept();
}
