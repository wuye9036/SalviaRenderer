#pragma once

#include <salvia/utility/api_symbols.h>

#include <string>
#include <vector>

namespace salvia::utility {

std::string SALVIA_UTILITY_API find_path(std::string const &relative_path);
std::string SALVIA_UTILITY_API find_path(std::string const &relative_path,
                                         std::vector<std::string> const &candidates);
std::vector<std::string> SALVIA_UTILITY_API default_search_paths();

} // namespace salvia::utility
