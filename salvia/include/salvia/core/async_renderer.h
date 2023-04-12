#pragma once

#include <salvia/core/renderer.h>

#include <eflib/utility/shared_declaration.h>

#include <any>
#include <functional>
#include <memory>
#include <vector>

namespace salvia::core {

EFLIB_DECLARE_CLASS_SHARED_PTR(renderer);

renderer_ptr create_async_renderer();

}  // namespace salvia::core
