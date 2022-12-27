#pragma once

#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

#include <type_traits>
#include <variant>

namespace eflib::composition {

template <class Value> struct indirect_ {
  indirect_(Value const &v) : value_{boost::make_local_shared<Value>(v)} {}
  indirect_(Value &&v) : value_{boost::make_local_shared<Value>(std::move(v))} {}

  operator Value &() & { return *value_; }

  Value &get() const { return *value_; }

private:
  boost::local_shared_ptr<Value> value_;
};

template <typename DATuple, typename IATuple> struct make_variant;

template <typename... Args> struct directs;
template <typename... Args> struct indirects;

template <typename... DirectArgs, typename... IndirectArgs>
struct make_variant<directs<DirectArgs...>, indirects<IndirectArgs...>> {
  using type = std::variant<DirectArgs..., indirect_<IndirectArgs>...>;
};

// Usage:
// using expression = make_variant_t<directs<literal_int, literal_float, literal_string>
//                                   indirects<unary_expression, binary_expression>>;
// that will expands to:
//
// std::variant<
//      literal_int, literal_float, literal_string,
//      indirect_<unary_expression>, indirect_<binary_expression>>;
template <typename D, typename I> using make_variant_t = typename make_variant<D, I>::type;

template <typename... Ts> struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;

}; // namespace eflib::composition