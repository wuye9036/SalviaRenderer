#include <gtest/gtest.h>

#include <sasl/parser/generator_next.h>

using namespace sasl::parser_next;

TEST(sasl_parser, next_compilation) {
  sasl::parser_next::any_attribute na;
  sasl::parser_next::queue_attribute qa;
  sasl::parser_next::sequence_attribute sa;
  sasl::parser_next::terminal_attribute ta;

  static_assert(attribute<decltype(na)>);
  static_assert(attribute<decltype(qa)>);
  static_assert(!attribute<nil_attribute>);
}

TEST(sasl_parser, next_get_child) {
  terminal_attribute ta{{0, token::make_empty(), token::make_empty()}};

  sequence_attribute sa{{0, token::make_empty(), token::make_empty()}, {{ta}, {ta}, {ta}}};
  EXPECT_EQ(sa.children.size(), 3);
}

TEST(sasl_parser, terminal_parser) {
  constexpr size_t TEST_TOKEN_ID = 121;

  terminal_p tp{.token_id = TEST_TOKEN_ID};
  std::vector<token> tokens{
      token::make(TEST_TOKEN_ID, "TestToken", 1, 1, "TestFileName.hlsl", false)};

  auto chat = sasl::common::diag_chat::create();

  auto r = tp.parse(tokens, *chat);
  EXPECT_EQ(r.state, result_state::succeed);
  EXPECT_EQ(r.rng.begin(), tokens.end());
  EXPECT_EQ(r.rng.end(), tokens.end());

  bool is_terminal_attribute =
      std::visit<bool>(eflib::composition::overload{[](terminal_attribute&) { return true; },
                                                    [](auto&&) {
                                                      return false;
                                                    }},
                       r.attr);
  EXPECT_TRUE(is_terminal_attribute);
}