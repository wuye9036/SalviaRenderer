#pragma once

#include <salviau/include/salviau_forward.h>
#include <eflib/include/string/string.h>
#include <vector>

BEGIN_NS_SALVIAU();

std::_tstring SALVIAU_API find_file(std::_tstring const& relative_path);
std::_tstring SALVIAU_API find_file(std::_tstring const& relative_path, std::vector<std::_tstring> const& candidates);

END_NS_SALVIAU();
