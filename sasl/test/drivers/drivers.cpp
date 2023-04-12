#include <gtest/gtest.h>

#include <sasl/drivers/drivers_api.h>

using namespace sasl::drivers;

TEST(sasl_drivers, create_driver_instance) {
  compiler_ptr out;
  sasl_create_compiler(out);
  EXPECT_TRUE(out != nullptr);
}