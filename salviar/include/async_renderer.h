#ifndef SALVIAR_COMMAND_BUFFER_H
#define SALVIAR_COMMAND_BUFFER_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/renderer.h>

#include <eflib/include/utility/shared_declaration.h>

#include <any>
#include <vector>
#include <memory>
#include <functional>

namespace salviar{

EFLIB_DECLARE_CLASS_SHARED_PTR(renderer);

renderer_ptr create_async_renderer();

}

#endif