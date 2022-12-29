#pragma once

#include <eflib/platform/config.h>

#include <salvia/utility/api_symbols.h>
#include <salvia/utility/common/gui.h>

namespace salvia::utility {

SALVIA_UTILITY_API gui *create_win_gui();
SALVIA_UTILITY_API void delete_win_gui(gui *app);

}
