#include <gtest/gtest.h>

#include <eflib/utility/extension.h>

struct TestMemFn {
  void foo(int) {}

  auto get_foo() {
    return &EF_THIS_MEM_FN(foo);
  }

  auto get_foo_const() const{
    return &EF_THIS_MEM_FN(foo);
  }
};

TEST(eflib_utility, extension) {
  [[maybe_unused]] TestMemFn instance_;
}