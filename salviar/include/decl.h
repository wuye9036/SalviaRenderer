#pragma once

#include <salviar/include/salviar_forward.h>
#include <eflib/include/utility/shared_declaration.h>

BEGIN_NS_SALVIAR();

struct ps_output;
class  vs_output;
class  vs_input;

EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_vertex_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_pixel_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_blend_shader);

EFLIB_DECLARE_CLASS_SHARED_PTR(vertex_cache);
EFLIB_DECLARE_CLASS_SHARED_PTR(rasterizer);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(framebuffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
EFLIB_DECLARE_CLASS_SHARED_PTR(stream_assembler);

EFLIB_DECLARE_CLASS_SHARED_PTR(buffer_manager);
EFLIB_DECLARE_CLASS_SHARED_PTR(texture_manager);

EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(surface);
EFLIB_DECLARE_CLASS_SHARED_PTR(texture);
EFLIB_DECLARE_CLASS_SHARED_PTR(texture_2d);
EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);
EFLIB_DECLARE_CLASS_SHARED_PTR(device);
EFLIB_DECLARE_CLASS_SHARED_PTR(renderer);
EFLIB_DECLARE_CLASS_SHARED_PTR(clipper);

EFLIB_DECLARE_CLASS_SHARED_PTR(raster_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(depth_stencil_state);

END_NS_SALVIAR();