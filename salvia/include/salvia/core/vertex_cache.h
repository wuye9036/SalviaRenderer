#ifndef SALVIAR_VERTEX_CACHE_H
#define SALVIAR_VERTEX_CACHE_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>

#include <eflib/memory/atomic.h>
#include <salvia/core/renderer.h>
#include <salviar/include/index_fetcher.h>

#include <boost/pool/pool.hpp>
#include <eflib/platform/boost_begin.h>
#include <eflib/platform/boost_end.h>

#include <utility>
#include <vector>

namespace salvia::core {

class stream_assembler;
struct render_stages;
struct render_state;

typedef size_t cache_entry_index;

EFLIB_DECLARE_CLASS_SHARED_PTR(vertex_cache);

class vertex_cache {
public:
  virtual void initialize(render_stages const *stages) = 0;
  virtual void update(render_state const *state) = 0;

  virtual void prepare_vertices() = 0;
  virtual void fetch3(vs_output **v, cache_entry_index id, uint32_t thread_id) = 0;
  virtual void update_statistic() = 0;

  virtual ~vertex_cache() {}
};

vertex_cache_ptr create_default_vertex_cache();

} // namespace salvia::core

#endif