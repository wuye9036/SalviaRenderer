#ifndef SALVIAR_DECL_H
#define SALVIAR_DECL_H

#include <eflib/include/platform/config.h>
#include "handles.h"
#include <salviar/include/salviar_forward.h>
BEGIN_NS_SALVIAR()


// stages of pipeline
class renderer_mementor;

class renderer;
class renderer_impl;
class vertex_cache;
class vertex_shader;
class clipper;
class rasterizer;
class shader_object;
class pixel_shader;
class blend_shader;
class framebuffer;
class index_fetcher;
class stream_assembler;

// resources
class buffer_manager;
class texture_manager;

struct sampler_desc;

struct raster_desc;
class raster_state;

struct depth_stencil_op_desc;
struct depth_stencil_desc;
class depth_stencil_state;

class buffer;
class surface;
class texture;
class texture_2d;
class sampler;

class device;
struct device_info;

//internal structures of pipeline
class vs_input;
class vs_output;
struct vs_input_op;
struct vs_output_op;
struct ps_output;

//public structure
struct input_element_desc;
class input_layout;

struct viewport;

//handles
DECL_HANDLE(vertex_cache, h_vertex_cache);
DECL_HANDLE(vertex_shader, h_vertex_shader);
DECL_HANDLE(rasterizer, h_rasterizer);
DECL_HANDLE(shader_object, h_shader_code);
DECL_HANDLE(pixel_shader, h_pixel_shader);
DECL_HANDLE(blend_shader, h_blend_shader);
DECL_HANDLE(framebuffer, h_framebuffer);
DECL_HANDLE(input_layout, h_input_layout);
DECL_HANDLE(stream_assembler, h_stream_assembler);

DECL_HANDLE(buffer_manager, h_buffer_manager);
DECL_HANDLE(texture_manager, h_texture_manager);

DECL_HANDLE(buffer, h_buffer);
DECL_HANDLE(surface, h_surface);
DECL_HANDLE(texture, h_texture);
DECL_HANDLE(texture_2d, h_texture_2d);
DECL_HANDLE(sampler, h_sampler);
DECL_HANDLE(device, h_device);
DECL_HANDLE(renderer, h_renderer);
DECL_HANDLE(clipper, h_clipper);
DECL_HANDLE(renderer_mementor, h_renderer_mementor);

DECL_HANDLE(raster_state, h_rasterizer_state);
DECL_HANDLE(depth_stencil_state, h_depth_stencil_state);

END_NS_SALVIAR()

#endif