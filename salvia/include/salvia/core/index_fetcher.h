#pragma once

#include <salvia/core/decl.h>

#include "salvia/common/format.h"
#include <salvia/common/constants.h>

#include <eflib/platform/typedefs.h>

namespace salvia::core {

struct render_state;

class index_fetcher {
public:
  void update(render_state const *state);
  void fetch_indexes(uint32_t *indexes_of_prim, uint32_t *min_index, uint32_t *max_index,
                     uint32_t prim_id_beg, uint32_t prim_id_end);

private:
  resource::buffer *index_buffer_;
  format index_format_;
  primitive_topology prim_topo_;
  uint32_t start_addr_;
  uint32_t base_vert_;
  uint32_t stride_;
};

} // namespace salvia::core
