#include <gtest/gtest.h>

#include <eflib/utility/string_conv.h>

using namespace eflib;

TEST(eflib_utility, string_conv) {
  auto converted_from_u8 = eflib::u8tou16(+u8"z\u00df\u6c34\U0001f34c");
  EXPECT_EQ(converted_from_u8, u"zß水\U0001f34c");
}