#pragma once

#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <vector>
#include <tuple>

namespace eflib {
template <class T> inline constexpr void hash_combine(std::size_t &seed, const T &v) {
  using std::hash;

  hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <std::forward_iterator It>
constexpr void hash_combine(std::size_t &seed, It first, It last) {
  for (; first != last; ++first) {
    hash_combine(seed, *first);
  }
}

template <typename T> size_t do_hash(T const &v) {
  using std::hash;
  return hash<T>{}(v);
}

struct hash_range {
  template <std::forward_iterator It>
  constexpr size_t operator()(It first, It last) const noexcept {
    std::size_t seed = 0;
    hash_combine(seed, first, last);
    return seed;
  }

  template <std::ranges::range RangeT> constexpr size_t operator()(RangeT &&r) const noexcept {
    return operator()(std::ranges::begin(r), std::ranges::end(r));
  }
};

struct hash_tuple {
  template <typename T1, typename T2>
  constexpr size_t operator()(std::pair<T1, T2> const &pair) const noexcept {
    std::size_t seed = 0;
    hash_combine(seed, pair.first);
    hash_combine(seed, pair.second);
    return seed;
  }

  template <typename... ArgsT>
  constexpr size_t operator()(std::tuple<ArgsT...> const &t) const noexcept {
    return operator()(t, std::make_index_sequence<sizeof...(ArgsT)>{});
  }

  template <typename... ArgsT, size_t... ArgInts>
  constexpr size_t operator()(std::tuple<ArgsT...> const &t,
                              std::index_sequence<ArgInts...>) const noexcept {
    size_t seed = 0;
    (hash_combine(seed, std::get<ArgInts>(t)), ...);
    return seed;
  }
};

namespace hash_detail {
template <std::size_t I, typename T>
inline typename std::enable_if_t<(I == std::tuple_size<T>::value)> hash_combine_tuple(std::size_t &,
                                                                                      T const &) {}

template <std::size_t I, typename T>
inline typename std::enable_if_t<(I < std::tuple_size<T>::value)>
hash_combine_tuple(std::size_t &seed, T const &v) {
  hash_combine(seed, std::get<I>(v));
  hash_combine_tuple<I + 1>(seed, v);
}

template <typename T> inline std::size_t hash_tuple(T const &v) {
  std::size_t seed = 0;
  hash_combine_tuple<0>(seed, v);
  return seed;
}
} // namespace hash_detail
} // namespace eflib
