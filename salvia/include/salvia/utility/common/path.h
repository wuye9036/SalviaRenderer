#pragma once

#include <salvia/utility/api_symbols.h>

#include <vector>
#include <string>

namespace salvia::utility {

std::string SALVIA_UTILITY_API find_path(std::string const &relative_path);
std::string SALVIA_UTILITY_API find_path(std::string const &relative_path,
                                  std::vector<std::string> const &candidates);

}
