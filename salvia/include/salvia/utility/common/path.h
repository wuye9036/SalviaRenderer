#pragma once

#include <eflib/string/string.h>
#include <salviau/include/salviau_forward.h>
#include <vector>

BEGIN_NS_SALVIAU();

std::string SALVIAU_API find_path(std::string const &relative_path);
std::string SALVIAU_API find_path(std::string const &relative_path,
                                  std::vector<std::string> const &candidates);
std::wstring SALVIAU_API find_path(std::wstring const &relative_path);
std::wstring SALVIAU_API find_path(std::wstring const &relative_path,
                                   std::vector<std::wstring> const &candidates);

END_NS_SALVIAU();
