#pragma once

#include <eflib/math/math.h>
#include <eflib/platform/stdint.h>

#include <array>
#include <tuple>

namespace salvia::shader_constant {
struct empty {};

using shader_constant_types = std::tuple<empty,
                                         bool,
                                         int8_t,
                                         uint8_t,
                                         int16_t,
                                         uint16_t,
                                         int32_t,
                                         uint32_t,
                                         int64_t,
                                         uint64_t,
                                         std::string,
                                         std::wstring,
                                         eflib::vec2,
                                         eflib::vec3,
                                         eflib::vec4,
                                         eflib::mat44,
                                         std::vector<eflib::mat44>>;

template <class T, class Tuple>
struct index_in_tuple;

template <class T, class... Types>
struct index_in_tuple<T, std::tuple<T, Types...>> {
  static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct index_in_tuple<T, std::tuple<U, Types...>> {
  static const std::size_t value = 1 + index_in_tuple<T, std::tuple<Types...>>::value;
};

template <typename T>
struct type_encode {
  static constexpr size_t id = index_in_tuple<T, shader_constant_types>::value;
};

#include <eflib/meta/voidptr.inl>

// reg functions
////////////////////////////////////////////
template <class T>
inline bool assign_impl(voidptr lhs, const_voidptr rhs) {
  *(T*)lhs = *(const T*)rhs;
  return true;
}

template <>
inline bool assign_impl<empty>(voidptr /*lhs*/, const_voidptr /*rhs*/) {
  return false;
}

using assign_impl_functype = bool (*)(voidptr, const_voidptr);
extern std::array<assign_impl_functype, std::tuple_size<shader_constant_types>::value>
    assign_impl_table;

inline bool assign(voidptr lhs, const_voidptr rhs) {
  auto const func_id = lhs.get_id();
  if (rhs.get_id() == func_id) {
    auto assign_func = assign_impl_table[func_id];
    return assign_func(lhs, rhs);
  }
  return false;
}

namespace detail {
class container {
public:
  virtual void get(voidptr pval, size_t pos) = 0;
  virtual void set(const_voidptr val, size_t pos) = 0;
  virtual ~container() {}
};

template <class ContainerImpl, class ElemType>
class container_impl : public container {
  typedef ContainerImpl container_type;
  typedef ElemType element_type;

  container_type* pcont;

public:
  container_impl(container_type& cont) : pcont(&cont) {}

  virtual void get(voidptr pval, size_t pos) {
    element_type* pelem = voidptr_cast<element_type>(pval);
    if (pelem) {
      *pelem = (*pcont)[pos];
    }
  }

  virtual void set(const_voidptr pval, size_t pos) {
    const element_type* pelem = voidptr_cast<element_type>(pval);
    if (pelem) {
      (*pcont)[pos] = *pelem;
    }
  }
};
}  // namespace detail
}  // namespace salvia::shader_constant
