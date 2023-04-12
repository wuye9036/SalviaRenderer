#pragma once

#include <locale>
#include <string>

namespace eflib {

template <class Facet>
struct deletable_facet : Facet {
  template <class... Args>
  deletable_facet(Args&&... args) : Facet(std::forward<Args>(args)...) {}
  ~deletable_facet() {}
};

std::u16string u8tou16(std::u8string_view s) {
  std::wstring_convert<deletable_facet<std::codecvt<char16_t, char, std::mbstate_t>>, char16_t>
      conv16;
  return conv16.from_bytes(reinterpret_cast<char const*>(s.data()),
                           reinterpret_cast<char const*>(s.data() + s.size()));
}

std::wstring mb2wide(std::string s) {
  std::wstring_convert<deletable_facet<std::codecvt<wchar_t, char, std::mbstate_t>>, wchar_t>
      conv16;
  return conv16.from_bytes(reinterpret_cast<char const*>(s.data()),
                           reinterpret_cast<char const*>(s.data() + s.size()));
}

}  // namespace eflib