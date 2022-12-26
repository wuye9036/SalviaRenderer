#pragma once

#include <sasl/parser/diags.h>

#include <sasl/common/diag_chat.h>
#include <sasl/common/token.h>

#include <eflib/utility/composition.h>
#include <eflib/utility/enum.h>

#include <range/v3/view.hpp>

#include <optional>
#include <variant>

namespace sasl::parser_next {

using token = sasl::common::token;
using namespace eflib::composition;
using namespace sasl::parser::diags;

} // namespace sasl::parser_next

namespace sasl::parser_next::attributes {

struct any_attribute;

struct attribute_base {
  intptr_t rule_id;
  token token_beg, token_end;
};

struct nil_attribute : std::monostate {};

struct terminal_attribute {
  attribute_base base;
};

// *|+|- rule
struct select_attribute {
  attribute_base base;
  std::optional<indirect_<any_attribute>> attr;
  size_t selected_index;
};

// rule0 | rule1
struct sequence_attribute {
  attribute_base base;
  std::vector<any_attribute> children;
};

// rule0 >> rule1
// rule0 > rule1
struct queue_attribute {
  attribute_base base;
  std::vector<any_attribute> children;
};

struct any_attribute : std::variant<nil_attribute, terminal_attribute, select_attribute,
                                    sequence_attribute, queue_attribute> {};

template <typename T>
concept attribute = std::same_as<decltype(std::declval<T>().base), attribute_base> ||
                    std::same_as<T, any_attribute>;

template <typename AttrT>
concept aggregated_attribute =
    std::same_as<AttrT, sequence_attribute> || std::same_as<AttrT, queue_attribute>;

decltype(auto) get_child(aggregated_attribute auto &&attr, size_t i) {
  return std::forward<decltype(attr)>(attr).children[i];
}

} // namespace sasl::parser_next::attributes

// ... Parser combinations ...
namespace sasl::parser_next::combinators {

using namespace eflib::enum_operators;
using namespace sasl::parser_next::attributes;

namespace {
constexpr uint32_t _recover_failed_offset = 4;
constexpr uint32_t _recover_failed_mask = 0xF0;
constexpr uint32_t _expected_offset = 0;
constexpr uint32_t _expected_mask = 0x0F;
} // namespace

enum class result_state : uint32_t {
  succeed = 0,
  expected = 1,

  recovered = 0x1 << _recover_failed_offset,
  failed = 0x2 << _recover_failed_offset,

  expected_failed = expected | failed,
  recovered_expected_failed = expected | recovered
};

constexpr result_state recover(result_state rs) noexcept {
  using enum result_state;
  return rs == expected_failed ? recovered_expected_failed : recovered;
}

constexpr result_state get_worse(result_state lhs, result_state rhs) noexcept {
  return std::max(lhs, rhs);
}

constexpr result_state to_better(result_state lhs, result_state rhs) noexcept {
  return std::min(lhs, rhs);
}

constexpr bool is_worse(result_state lhs, result_state rhs) noexcept { return lhs > rhs; }

constexpr bool is_better(result_state lhs, result_state rhs) noexcept { return lhs < lhs; }

constexpr bool is_expected_failed_or_recovered(result_state rs) noexcept {
  using enum result_state;
  return (rs & _expected_mask) == expected;
}

constexpr bool is_continuable(result_state rs) noexcept {
  return (rs & _recover_failed_mask) != result_state::failed;
}

using token_range = ranges::subrange<std::vector<token>::iterator>;

template <attribute Attr> struct parsing_result {
  result_state state;
  token_range rng;
  Attr attr;
};

struct paser_base {
  bool is_expected = false;
};

struct terminal_p {
  parsing_result<attributes::any_attribute> parse(token_range rng, sasl::common::diag_chat &diags) {
    if (rng.empty()) {
      return {result_state::failed, rng, {nil_attribute{}}};
    }

    if (rng.front().id() == token_id) {
      return {result_state::succeed, rng.next(), terminal_attribute{{0, rng.front(), rng.front()}}};
    }

    diags.report(unmatched_token, rng.front().file_name(), rng.front().span(),
                 fmt::arg("syntax_error", rng.front().lit()));
    return {result_state::failed, rng, nil_attribute{}};
  }

  size_t token_id;
};

struct repeat_p {};
struct select_p {};
struct sequential_p {};
struct negative_p {};

struct end_p {};
struct error_catch_p {};

struct parser {};

struct named_p {
  std::string name;
};

template <typename T> struct is_parsing_result : std::false_type {};
template <attribute Attr> struct is_parsing_result<parsing_result<Attr>> : std::true_type {};

template <typename T> constexpr auto is_parsing_result_v = is_parsing_result<T>::value;

template <typename T>
concept range_p = is_parsing_result_v<T>;

#if 0
typedef std::function<parse_results(sasl::common::diag_chat * /*diags*/,
                                    token_iterator const & /*origin iter*/,
                                    token_iterator & /*current start iter*/)>
    error_handler;
class error_catcher;

class parser {
public:
  parser();
  virtual parse_results parse(token_iterator &iter, token_iterator end,
                              std::shared_ptr<attribute> &attr,
                              sasl::common::diag_chat *diags) const = 0;
  bool is_expected() const;
  void is_expected(bool v);
  error_catcher operator[](error_handler on_err);
  virtual std::shared_ptr<parser> clone() const = 0;
  virtual ~parser() {}

private:
  bool expected;
};

class terminal : public parser {
public:
  terminal(size_t tok_id, std::string const &desc);
  terminal(terminal const &rhs);
  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl::common::diag_chat *diags) const;
  std::shared_ptr<parser> clone() const;
  std::string const &get_desc() const;

private:
  terminal &operator=(terminal const &);
  size_t tok_id;
  std::string desc;
};

class repeater : public parser {

public:
  static size_t const unlimited;

  repeater(size_t lower_bound, size_t upper_bound, std::shared_ptr<parser> expr);
  repeater(repeater const &rhs);
  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl::common::diag_chat *diags) const;
  std::shared_ptr<parser> clone() const;

private:
  size_t lower_bound;
  size_t upper_bound;
  std::shared_ptr<parser> expr;
};

class selector : public parser {

public:
  selector();
  selector(selector const &rhs);

  selector &add_branch(std::shared_ptr<parser> p);
  std::vector<std::shared_ptr<parser>> const &branches() const;

  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl::common::diag_chat *diags) const;
  std::shared_ptr<parser> clone() const;

private:
  std::vector<std::shared_ptr<parser>> slc_branches;
};

class queuer : public parser {
public:
  queuer();
  queuer(queuer const &rhs);

  queuer &append(std::shared_ptr<parser> p, bool is_expected = false);
  std::vector<std::shared_ptr<parser>> const &exprs() const;

  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl::common::diag_chat *diags) const;
  std::shared_ptr<parser> clone() const;

private:
  std::vector<std::shared_ptr<parser>> exprlst;
};

class negnativer : public parser {
public:
  negnativer(std::shared_ptr<parser>);
  negnativer(negnativer const &rhs);

  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl::common::diag_chat *diags) const;
  std::shared_ptr<parser> clone() const;

private:
  std::shared_ptr<parser> expr;
};

class rule : public parser {
public:
  rule();
  rule(intptr_t id);
  rule(std::shared_ptr<parser> expr, intptr_t id = -1);
  rule(rule const &rhs);
  rule(parser const &rhs);
  rule &operator=(parser const &rhs);
  rule &operator=(rule const &rhs);

  intptr_t id() const;
  std::string const &name() const;
  void name(std::string const &v);
  parser const *get_parser() const;
  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl::common::diag_chat *diags) const;
  std::shared_ptr<parser> clone() const;

private:
  intptr_t preset_id;
  std::shared_ptr<parser> expr;
  std::string rule_name;
};

class rule_wrapper : public parser {
public:
  rule_wrapper(rule_wrapper const &rhs);
  rule_wrapper(rule const &rhs);
  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl::common::diag_chat *diags) const;
  std::shared_ptr<parser> clone() const;
  std::string const &name() const;
  rule const *get_rule() const;

private:
  rule_wrapper &operator=(rule_wrapper const &);
  rule const &r;
};

class endholder : public parser {
public:
  endholder();
  endholder(endholder const &);
  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl:: ::diag_chat *diags) const;
  std::shared_ptr<parser> clone() const;
};

class error_catcher : public parser {
public:
  error_catcher(std::shared_ptr<parser> const &p, error_handler err_handler);
  error_catcher(error_catcher const &);
  std::shared_ptr<parser> clone() const;
  parse_results parse(token_iterator &iter, token_iterator end, std::shared_ptr<attribute> &attr,
                      sasl::common::diag_chat *diags) const;

private:
  std::shared_ptr<parser> expr;
  error_handler err_handler;
};

// class exceptor: public parser
//{
// public:
//	exceptor( std::shared_ptr<parser> const& p );
//	exceptor( exceptor const& );
//	std::shared_ptr<parser> clone() const;
//	parse_results parse( token_iterator& iter, token_iterator end, std::shared_ptr<attribute>&
// attr, sasl::common::diag_chat* diags ) const; private: 	std::shared_ptr<parser>	expr;
// };
//////////////////////////////////////////////////////////////////////////
// Operators for building parser combinator.
repeater operator*(parser const &expr);
repeater operator-(parser const &expr);
selector operator|(parser const &expr0, parser const &expr1);
selector operator|(selector const &expr0, parser const &expr1);
selector operator|(selector const &expr0, selector const &expr1);
queuer operator>>(parser const &expr0, parser const &expr1);
queuer operator>>(queuer const &expr0, parser const &expr1);
queuer operator>(parser const &expr0, parser const &expr1);
queuer operator>(queuer const &expr0, parser const &expr1);
negnativer operator!(parser const &expr1);
#endif
} // namespace sasl::parser_next::combinators

namespace sasl::parser_next {
using namespace attributes;
using namespace combinators;
} // namespace sasl::parser_next