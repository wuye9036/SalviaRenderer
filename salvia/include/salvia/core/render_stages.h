#pragma once

#include <eflib/utility/shared_declaration.h>

namespace salvia::core {

EFLIB_DECLARE_CLASS_SHARED_PTR(stream_assembler);
EFLIB_DECLARE_CLASS_SHARED_PTR(geom_setup_engine);
EFLIB_DECLARE_CLASS_SHARED_PTR(rasterizer);
EFLIB_DECLARE_CLASS_SHARED_PTR(vertex_cache);
EFLIB_DECLARE_CLASS_SHARED_PTR(framebuffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(host);

struct render_stages {
  stream_assembler_ptr assembler;
  geom_setup_engine_ptr gse;
  vertex_cache_ptr vert_cache;
  rasterizer_ptr ras;
  framebuffer_ptr backend;
  host_ptr host;
};

} // namespace salvia::core
