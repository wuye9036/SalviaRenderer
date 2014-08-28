#pragma once

#include <eflib/include/platform/config.h>

#include <salviau/include/salviau_forward.h>
#include <salviau/include/common/application.h>

BEGIN_NS_SALVIAU();

SALVIAU_API gui* create_win_gui();
SALVIAU_API void delete_win_gui(gui* app);

END_NS_SALVIAU();
