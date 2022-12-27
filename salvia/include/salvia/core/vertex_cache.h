#pragma once

#include <salvia/core/decl.h>
#include <salvia/core/renderer.h>
#include <salvia/core/index_fetcher.h>

#include <eflib/concurrency/atomic.h>

#include <boost/pool/pool.hpp>

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
  virtual void fetch3(shader::vs_output **v, cache_entry_index id, uint32_t thread_id) = 0;
  virtual void update_statistic() = 0;

  virtual ~vertex_cache() {}
};

vertex_cache_ptr create_default_vertex_cache();

} // namespace salvia::core
