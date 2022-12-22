#include <gtest/gtest.h>

#include <sasl/parser/generator_next.h>

using namespace sasl::parser_next;

TEST(sasl_parser, next_compilation) {
  sasl::parser_next::attribute na;
  sasl::parser_next::queue_attribute qa;
  sasl::parser_next::sequence_attribute sa;
  sasl::parser_next::terminal_attribute ta;
}


TEST(sasl_parser, next_get_child) {
  terminal_attribute ta{0, token::make_empty(), token::make_empty()};

  sequence_attribute sa{0, token::make_empty(), token::make_empty(), {{ta}, {ta}, {ta}}};
  EXPECT_EQ(sa.children.size(), 3);
}