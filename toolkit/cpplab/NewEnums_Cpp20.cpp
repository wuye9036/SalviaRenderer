#include <gtest/gtest.h>

#include <array>


struct bit_range {
  uint32_t begin{0};
  uint32_t end{0};

  [[nodiscard]] constexpr bool empty() const { return begin == end; }

  template <std::integral T>
  constexpr T value_mask() {
    return (T{1} << (end - begin)) - 1;
  };

  template <std::integral T>
  constexpr T mask() {
    return value_mask<T>() << begin;
  }

  template <std::integral T>
  constexpr T value(T value) {
    return (value & value_mask<T>()) << begin;
  }
};

constexpr bit_range next_bits(bit_range const& prev, uint32_t length) {
  return bit_range{prev.end, prev.end + length};
}

constexpr bit_range lo(bit_range const& rng, uint32_t length) {
  return {rng.begin, length};
}

constexpr bit_range hi(bit_range const& rng, uint32_t length) {
  return {rng.end - length, rng.end};
}

constexpr bit_range mid(bit_range const& rng, uint32_t begin, uint32_t end) {
  return {rng.begin + begin, rng.end - end};
}

template <std::integral T>
constexpr T bits_value_mask(bit_range const& rng) {
  return (T{1} << (rng.end - rng.begin)) - 1;
}

template <std::integral T>
constexpr T bits_mask(bit_range const& rng) {
  return ((T{1} << (rng.end - rng.begin)) - 1) << rng.begin;
}

template <std::integral T, typename... Args>
constexpr T bits_value(bit_range b_rng, T value, Args&&... args) {
  if constexpr (sizeof...(args) == 0)
    return b_rng.value<T>(value);
  else
    return b_rng.value<T>(value) | bits(std::forward<Args>(args)...);
}

struct bits_layout_trait {};

template <typename Tag, typename... SubLayout>
struct bits_layout : bits_layout_trait {
  bool is_union;
  Tag tag;
  uint32_t width;
  std::tuple<SubLayout...> sub_layouts;

  template <typename Tag2>
  [[nodiscard]] constexpr bit_range bits(Tag2 t) const noexcept {
    if (t == tag) {
      return bit_range{0, width};
    }

    auto sub_offset = 0;
    constexpr auto sub_size = std::tuple_size_v<decltype(sub_layouts)>;

    if constexpr (sub_size == 0) {
      return bit_range{};
    }

    auto sub_ranges = std::apply(
        [t](auto... subs) {
          return std::array<bit_range, sub_size>{subs.bits(t)...};
        },
        sub_layouts);

    auto sub_width = std::apply(
        [](auto... subs) { return std::array<uint32_t, sub_size>{subs.width...}; },
        sub_layouts);

    for (size_t i = 0; i < sub_size; ++i) {
      auto sub_range = sub_ranges[i];
      if (!sub_range.empty()) {
        return bit_range{sub_range.begin + sub_offset, sub_range.end + sub_offset};
      }
      if (!is_union) {
        sub_offset += sub_width[i];
      }
    }

    return bit_range{0, 0};
  }
};

template <auto tag>
constexpr auto bit_s(std::derived_from<bits_layout_trait> auto... sub_layouts) {
  return bits_layout<decltype(tag), decltype(sub_layouts)...>{
      {}, false, tag, (sub_layouts.width + ...), {sub_layouts...}};
}

template <auto tag>
constexpr auto bit_u(std::derived_from<bits_layout_trait> auto... sub_layouts) {
  return bits_layout<decltype(tag), decltype(sub_layouts)...>{
      {}, true, tag, std::max({sub_layouts.width...}), {sub_layouts...}};
}

constexpr auto def_(auto tag, uint32_t width) {
  return bits_layout<decltype(tag)>{{}, false, tag, width, {}};
}


TEST(NewEnums, NewEnums) {
  enum tags { t, t0, t1, t00, t01, t10, t11 };

  {
    constexpr auto layout0 = bit_s<t>(def_(t0, 8));
    constexpr auto t0_bits_layout0 = layout0.bits(t0);
    static_assert(t0_bits_layout0.begin == 0);
    static_assert(t0_bits_layout0.end == 8);
  }

  {
    constexpr auto layout1 = bit_s<t>(def_(t0, 8), def_(t1, 8));
    static_assert(layout1.width == 16);
    static_assert(!layout1.is_union);

    constexpr auto t0_bits_layout1 = layout1.bits(t0);
    static_assert(t0_bits_layout1.begin == 0);
    static_assert(t0_bits_layout1.end == 8);

    constexpr auto t1_bits_layout1 = layout1.bits(t1);
    static_assert(t1_bits_layout1.begin == 8);
    static_assert(t1_bits_layout1.end == 16);
  }

  {
    // clang-format off
    constexpr auto layout_ssu =
        bit_s<t>(
          bit_s<t0>(
            def_(t00, 8),
            def_(t01, 8)),
          bit_u<t1>(
            def_(t10, 10),
            def_(t11, 14)));
    // clang-format on
    static_assert(layout_ssu.width == 30);

    static_assert(layout_ssu.bits(t0).begin == 0);
    static_assert(layout_ssu.bits(t0).end == 16);
    static_assert(layout_ssu.bits(t11).begin == 16);
    static_assert(layout_ssu.bits(t11).end == 30);

    constexpr auto t10_bits = layout_ssu.bits(t10);
    static_assert(t10_bits.begin == 16);
    static_assert(t10_bits.end == 26);

    constexpr auto t00_bits = layout_ssu.bits(t00);
    static_assert(bits_mask<uint32_t>(t00_bits) == 0xFF);
    static_assert(bits_mask<uint32_t>(layout_ssu.bits(t01)) == 0xFF00);

    constexpr auto t10_v = 0b1011;
    static_assert(bits_value(t10_bits, t10_v) == (t10_v << 16));
    static_assert(bits_value(t10_bits, 1U) == (1U << 16));
    static_assert(bits_mask<uint32_t>(t10_bits) == ((1U << 26) - (1U << 16)));

  }
}

// template <size_t size>
// struct uint_of {
//   static consteval auto gen_value_() {
//     if constexpr (size == 1) {
//       return std::uint8_t{size};
//     } else if constexpr (size == 2) {
//       return std::uint16_t{size};
//     } else if constexpr (size == 4) {
//       return std::uint32_t{size};
//     } else if constexpr (size == 8) {
//       return std::uint64_t{size};
//     } else {
//       return;
//     }
//   }
//
//   using type = decltype(gen_value_());
// };
//
// struct builtin_type_layout {
//   uint32_t dim_count : 2 {0};
//   uint32_t bytes_log2 : 4 {0};
//   uint32_t sign : 1 {0};
//   uint32_t type_class : 2 {0};
//   uint32_t dim0 : 8 {0};
//   uint32_t dim1 : 8 {0};
//   uint32_t reserved : 7 {0};
// };
//
// template <typename T>
// union layout_conv {
//   T layout;
//   typename uint_of<sizeof(T)>::type value;
// };

//
// consteval auto to_integral(builtin_type_layout l) {
//  auto c = reinterpret_cast<uint32_t const*>
//  return c;
//}
//
// consteval builtin_type_layout from_integral(uint32_t v) {
//  return layout_conv<builtin_type_layout>{.value=v}.layout;
//}
//
// enum type_classes { integers = 0, floats = 1, booleans = 2 };
//
// enum class builtin_types : uint32_t {
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
// struct Agg{
//  union {
//    uint32_t a: 8;
//    struct {
//      uint32_t b: 4;
//      uint32_t c: 4;
//    } d;
//  };
//};
//
