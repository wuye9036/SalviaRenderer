#pragma once

#include <eflib/include/platform/config.h>

#include <salviau/include/salviau_forward.h>
#include <salviau/include/common/application.h>

BEGIN_NS_SALVIAU();

SALVIAU_API application* create_win_application();
SALVIAU_API void		 delete_win_application(application* app);

END_NS_SALVIAU();
