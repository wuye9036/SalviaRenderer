#include <gtest/gtest.h>

// example:
struct bit_range {
  uint32_t begin;
  uint32_t end;
};

constexpr bit_range next_bits(bit_range const &prev, uint32_t length) {
  return bit_range{prev.end, prev.end + length};
}

constexpr bit_range lo(bit_range const &rng, uint32_t length) { return {rng.begin, length}; }

constexpr bit_range hi(bit_range const &rng, uint32_t length) {
  return {rng.end - length, rng.end};
}

constexpr bit_range mid(bit_range const &rng, uint32_t begin, uint32_t end) {
  return {rng.begin + begin, rng.end - end};
}

template <std::integral T> constexpr T mask(bit_range const &rng) {
  return (T{1} << (rng.end - rng.begin)) - 1;
}

template <std::integral T, typename... Args>
constexpr T bits(bit_range tag, T value, Args &&...args) {
  if constexpr (sizeof...(args) == 0)
    return (value & mask<T>(tag)) << tag.begin;
  else
    return ((value & mask<T>(tag)) << tag.begin) | bits(std::forward<Args>(args)...);
}

template <std::integral T> constexpr T bits(T value) { return value; }

constexpr auto builtin_bits = bit_range{0, 32};

constexpr auto dim_count = hi(builtin_bits, 2);

constexpr std::integral auto scalar = bits(dim_count, 0U);
constexpr std::integral auto vector = bits(dim_count, 1U);
constexpr std::integral auto matrix = bits(dim_count, 2U);

//

//enum tags { t, t0, t1, t00, t01, t10, t11 };
//
//const auto layout = bit_s<t>(bit_s<t0>(t00, 8, t01, 8), bit_u<t1>(t10, 10, t11, 14));
//
//struct bits_layout_trait {};
//
//template <typename Tag, typename... Sub> struct bits_layout : bits_layout_trait {
//  Tag tag;
//  uint32_t begin;
//  uint32_t end;
//  std::tuple<Sub...> sub_layouts;
//};
//
//template <auto tag> constexpr auto bit_s(std::derived_from<bits_layout_trait> auto... sub_layouts) {
//  return bit_layouts<tag, decltype(sub_layouts)...>{tag, 0, 0, {sub_layouts...}};
//}
//
//constexpr auto get_range(std::derived_from<bits_layout_trait> auto const &layout, auto tag) {
//  if constexpr (std::same_as<decltype(layout), bits_layout_trait>) {
//    return bit_range{layout.begin, layout.end};
//  } else {
//    if constexpr (std::same_as<decltype(layout.tag), decltype(tag)>) {
//      return bit_range{layout.begin, layout.end};
//    } else {
//      return get_range(layout.sub_layouts, tag);
//    }
//  }
//}
//
//auto t0_bit_range = get_range(layout, t0);
//auto value0 = bit_value(layout, t0, 1);
//auto value1 = bit_value(t0_bit_range, 1);
//auto value2 = t0_bit_range.value(1);

template <size_t size>
struct uint_of {
  static consteval auto gen_value_() {
    if constexpr (size == 1) {
      return std::uint8_t{};
    } else if constexpr (size == 2) {
      return std::uint16_t{};
    } else if constexpr (size == 4) {
      return std::uint32_t{};
    } else if constexpr (size == 8) {
      return std::uint64_t{};
    } else {
      return;
    }
  }

  using type = decltype(gen_value_());
};

struct builtin_type_layout {
  uint32_t dim_count : 2 {0};
  uint32_t bytes_log2 : 4 {0};
  uint32_t sign : 1 {0};
  uint32_t type_class : 2 {0};
  uint32_t dim0 : 8 {0};
  uint32_t dim1 : 8 {0};
  uint32_t reserved : 7 {0};
};

template <typename T>
union layout_conv {
  T layout;
  typename uint_of<sizeof(T)>::type value;
};

//
//consteval auto to_integral(builtin_type_layout l) {
//  auto c = reinterpret_cast<uint32_t const*>
//  return c;
//}
//
//consteval builtin_type_layout from_integral(uint32_t v) {
//  return layout_conv<builtin_type_layout>{.value=v}.layout;
//}
//
//enum type_classes { integers = 0, floats = 1, booleans = 2 };
//
//enum class builtin_types : uint32_t {
//  none = 0,
//
//  sint64  = to_integral({.bytes_log2 = 3, .sign = 1, .type_class = integers}),
//  sint32  = to_integral({.bytes_log2 = 2, .sign = 1, .type_class = integers}),
//  sint16  = to_integral({.bytes_log2 = 1, .sign = 1, .type_class = integers}),
//  sint8   = to_integral({.bytes_log2 = 0, .sign = 1, .type_class = integers}),
//
//  uint64  = to_integral({.bytes_log2 = 3, .sign = 0, .type_class = integers}),
//  uint32  = to_integral({.bytes_log2 = 2, .sign = 0, .type_class = integers}),
//  uint16  = to_integral({.bytes_log2 = 1, .sign = 0, .type_class = integers}),
//  uint8   = to_integral({.bytes_log2 = 0, .sign = 0, .type_class = integers}),
//
//  float16 = to_integral({.bytes_log2 = 1, .sign = 1, .type_class = floats}),
//  float32 = to_integral({.bytes_log2 = 2, .sign = 1, .type_class = floats}),
//  float64 = to_integral({.bytes_log2 = 3, .sign = 1, .type_class = floats}),
//
//  boolean = to_integral({.bytes_log2 = 0, .sign = 0, .type_class = booleans})
//};
//
//struct Agg{
//  union {
//    uint32_t a: 8;
//    struct {
//      uint32_t b: 4;
//      uint32_t c: 4;
//    } d;
//  };
//};
//
//TEST(NewEnums, NewEnums) {
//  static_assert(sizeof(Agg) == 1);
//}