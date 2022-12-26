#pragma once

#include <salviar/include/salviar_forward.h>

#include <salvia/common/constants.h>
#include <salviar/include/async_object.h>

#include <eflib/memory/pool.h>

#include <memory>

namespace salvia::core {

struct vs_output_op;
class vs_output;
class vertex_cache;
struct viewport;
struct thread_context;

struct geom_setup_context {
  vs_output_op const *vso_ops;
  vertex_cache *dvc;
  prim_type prim;
  size_t prim_size;
  size_t prim_count;
  bool (*cull)(float area);

  async_object *pipeline_stat;
  accumulate_fn<uint64_t>::type acc_cinvocations;
};

/*
        Primitive Verts -> Primitive Packages -> Clipped Packages (Sparse) -> Compacted Clipped
   Primitive Verts
*/
class geom_setup_engine {

public:
  geom_setup_engine();

  void execute(geom_setup_context const *, uint64_t (*fetch_time_stamp)());

  size_t verts_count() const {
    return static_cast<size_t>(clipped_package_compacted_addresses_[clipping_package_count_]);
  }

  vs_output **verts() const { return compacted_verts_.get(); }

  uint64_t compact_start_time() const { return compact_start_time_; }

private:
  typedef eflib::pool::reserved_pool<vs_output> vs_output_pool;

  void clip_geometries();
  void compact_geometries();

  void threaded_clip_geometries(thread_context const *thread_ctx);
  void threaded_compact_geometries(thread_context const *thread_ctx);

  std::shared_ptr<vs_output_pool[]> vso_pools_;

  std::shared_ptr<vs_output *[]> clipped_verts_;
  size_t clipped_verts_cap_;

  std::shared_ptr<uint32_t[]> clipped_package_verts_count_;
  size_t clipped_package_verts_count_cap_;

  std::shared_ptr<vs_output *[]> compacted_verts_;
  size_t compacted_verts_cap_;

  std::shared_ptr<uint32_t[]> clipped_package_compacted_addresses_;
  size_t clipped_package_compacted_addresses_cap_;

  int32_t clipping_package_count_;

  size_t thread_count_;
  uint64_t compact_start_time_;

  geom_setup_context const *ctxt_;
};

} // namespace salvia::core
