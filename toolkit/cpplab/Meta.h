#pragma once

#include <type_traits>
#include <optional>

// Utilities
struct nil {};

template <typename T, template <typename...> class Tmpl>
struct is_specialized_impl : std::false_type {};
template <template <typename...> class Tmpl, typename... Ps>
struct is_specialized_impl<Tmpl<Ps...>, Tmpl> : std::true_type {};

template <typename T, template <typename...> class Tmpl>
concept is_specialized = is_specialized_impl<T, Tmpl>::value;

template <typename T>
concept is_optional = is_specialized<T, std::optional>;

template <typename T1, typename T2> struct combine_composition {};

template <template <typename...> class C, typename... T1s, typename... T2s>
struct combine_composition<C<T1s...>, C<T2s...>> {
  using type = C<T1s..., T2s...>;
};

template <template <typename...> class C, typename... Ts>
struct combine_composition<C<nil>, C<Ts...>> {
  using type = C<Ts...>;
};

template <typename T1, typename T2>
using combine_composible_t = typename combine_composition<T1, T2>::type;

template <typename U, typename T> decltype(auto) cast_ref(T &v) { return ((U &)v); }

template <typename U, typename T> decltype(auto) cast_ref(T const &v) { return ((U const &)v); }

template <typename U, typename T> decltype(auto) cast_ref(T &&v) { return ((U &&) v); }
