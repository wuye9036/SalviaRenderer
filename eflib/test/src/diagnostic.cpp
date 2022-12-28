#include <gtest/gtest.h>

#include <eflib/diagnostics/assert.h>

TEST(eflib_diagnostic, complation_of_assertions) {
  [[maybe_unused]] auto p = []() {
    ef_verify(true);
    ef_verify(false);
    ef_unimplemented();
    ef_unreachable("Unreachable");
    ef_unreachable("Unreachable {}", 0);
  };
}