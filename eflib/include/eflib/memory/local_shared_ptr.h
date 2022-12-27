#pragma once

namespace eflib {
template <typename T> struct local_shared_ptr : storage_policy<T> {
  local_shared_ptr<>

      private : size_t ref_count_;
  T *data_;
};
} // namespace eflib