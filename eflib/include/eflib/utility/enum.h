#pragma once

#include <eflib/platform/typedefs.h>

#include <type_traits>
#include <utility>

namespace eflib {

namespace detail {
namespace { // avoid ODR-violation
template <class T> auto test_sizable(int) -> decltype(sizeof(T), std::true_type{});
template <class> auto test_sizable(...) -> std::false_type;

template <class T>
auto test_nonconvertible_to_int(int)
    -> decltype(static_cast<std::false_type (*)(int)>(nullptr)(std::declval<T>()));
template <class> auto test_nonconvertible_to_int(...) -> std::true_type;

template <class T>
constexpr bool is_scoped_enum_impl =
    std::conjunction_v<decltype(test_sizable<T>(0)), decltype(test_nonconvertible_to_int<T>(0))>;
} // namespace
} // namespace detail

template <class> struct is_scoped_enum : std::false_type {};

template <class E>
  requires std::is_enum_v<E>
struct is_scoped_enum<E> : std::bool_constant<detail::is_scoped_enum_impl<E>> {};

template <class E> inline constexpr bool is_scoped_enum_v = is_scoped_enum<E>::value;

template <typename E>
concept EnumType = is_scoped_enum_v<E>;

template <EnumType E> constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

template <EnumType E> constexpr auto e2i(E e) noexcept { return to_underlying(e); }

namespace enum_operators {

template <EnumType E> constexpr E operator|(E lhs, E rhs) noexcept {
  auto lhs_v = to_underlying(lhs);
  auto rhs_v = to_underlying(rhs);
  return static_cast<E>(lhs_v | rhs_v);
}

template <EnumType E> constexpr E operator&(E lhs, E rhs) noexcept {
  auto lhs_v = to_underlying(lhs);
  auto rhs_v = to_underlying(rhs);
  return static_cast<E>(lhs_v & rhs_v);
}

template <EnumType E> constexpr E operator^(E lhs, E rhs) noexcept {
  auto lhs_v = to_underlying(lhs);
  auto rhs_v = to_underlying(rhs);
  return static_cast<E>(lhs_v ^ rhs_v);
}

template <EnumType E> constexpr E operator~(E e) noexcept {
  auto e_v = to_underlying(e);
  return static_cast<E>(~e_v);
}

constexpr auto operator<<(std::integral auto lhs, EnumType auto rhs) noexcept {
  return lhs << to_underlying(rhs);
}

constexpr auto operator>>(std::integral auto lhs, EnumType auto rhs) noexcept {
  return lhs >> to_underlying(rhs);
}

constexpr auto operator<<(EnumType auto lhs, std::integral auto rhs) noexcept {
  return static_cast<decltype(lhs)>(to_underlying(lhs) << rhs);
}

constexpr auto operator>>(EnumType auto lhs, std::integral auto rhs) noexcept {
  return static_cast<decltype(lhs)>(to_underlying(lhs) >> rhs);
}

} // namespace enum_operators
} // namespace eflib