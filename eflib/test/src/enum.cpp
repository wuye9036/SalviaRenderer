#include <gtest/gtest.h>

#include <eflib/utility/enum.h>

using namespace eflib;

enum class test_ec { a = 0b101 };
enum class test_ec_i : int { a = 0b101 };
enum test_e { e_a = 0b101 };
TEST(eflib_utility, enums) {
  static_assert(is_scoped_enum_v<test_ec>);
  static_assert(is_scoped_enum_v<test_ec_i>);
  static_assert(!is_scoped_enum_v<test_e>);
  static_assert(!is_scoped_enum_v<int>);

  using zero_type = decltype(0u);
  static_assert(EnumOrIntType<int>);
  static_assert(EnumOrIntType<zero_type>);

  static_assert(std::is_same_v<decltype(to_underlying_or_fwd(0u)), decltype(0u)>);
}

TEST(eflib_utility, enum_ops) {
  using namespace eflib::enum_operators;
  EXPECT_EQ(test_ec::a, test_ec::a & 0xFFU);
  EXPECT_NE(test_ec::a, test_ec::a | 0xFFU);
  EXPECT_NE(test_ec::a, test_ec::a & 0x00U);
  EXPECT_EQ(test_ec::a, test_ec::a | 0x00U);
}