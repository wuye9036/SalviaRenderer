#include <gtest/gtest.h>

#include <sasl/common/token.h>

using namespace sasl::common;

namespace test_common {
TEST(sasl_common, token_construct_move_copy) {
  std::string const s{"test_string"};

  token t0 = token::make(s);
  EXPECT_EQ(t0.lit(), s);
  EXPECT_FALSE(t0.is_uninitialized());
  EXPECT_TRUE(t0.is_valid());

  token t1;
  EXPECT_TRUE(t1.is_uninitialized());
  EXPECT_FALSE(t1.is_valid());

  token t2 = token::make_empty();
  EXPECT_FALSE(t2.is_uninitialized());
  EXPECT_FALSE(t2.is_valid());

  token t3{std::move(t0)};
  EXPECT_FALSE(t3.is_uninitialized());
  EXPECT_TRUE(t3.is_valid());
  EXPECT_EQ(t3.lit(), s);
  EXPECT_TRUE(t0.is_uninitialized());

  token t4 = t3;
  EXPECT_EQ(t4.lit(), s);
  EXPECT_TRUE(t4.is_valid());
  EXPECT_EQ(t4.lit().data(), t3.lit().data());
}

TEST(sasl_common, token_ref_counting) {
  std::string const s{"test_string"};

  token t0 = token::make(s);
  token t1 = token::make(s);
}

} // namespace test_common