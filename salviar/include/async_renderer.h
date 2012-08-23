#ifndef SALVIAR_COMMAND_BUFFER_H
#define SALVIAR_COMMAND_BUFFER_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/renderer.h>
#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(renderer);

renderer_ptr create_async_renderer();

END_NS_SALVIAR();

#endif