#include <salvia/resource/resource_manager.h>

#include <salvia/resource/mapped_resource.h>

namespace salvia::resource {

template <typename T>
result resource_manager::map_impl(mapped_resource &mapped, T const &res, map_mode mm) {
  if (!res) {
    return result::invalid_parameter;
  }

  if (map_mode_ != map_mode_none) {
    return result::failed;
  }

  switch (mm) {
  case map_mode_none:
    return result::failed;

  case map_read:
  case map_read_write:
    renderer_sync_(); // Mapping when renderer is flushed.
    break;

  case map_write:
  case map_write_discard:
  case map_write_no_overwrite:
    // Do not need to synchronize yet.
    break;
  }

  result ret = res->map(mapped_resource_, mm);
  if (ret != result::ok) {
    return ret;
  }

  // If return address is actual buffer, we need to sync renderer.
  // Otherwise, 'sync' will be delayed to unlock.
  if (mapped_resource_.data == mapped_data_.data()) {
    switch (mm) {
    case map_write:
    case map_write_discard:
      renderer_sync_();
      break;
    default:
      // Do nothing.
      break;
    }
  }

  mapped.data = mapped_resource_.data;
  mapped.row_pitch = mapped_resource_.row_pitch;
  mapped.depth_pitch = mapped_resource_.depth_pitch;

  return ret;
}

result resource_manager::map(mapped_resource &mapped, buffer_ptr const &buf, map_mode mm) {
  result ret = map_impl(mapped, buf, mm);
  if (ret == result::ok) {
    mapped_buf_ = buf;
  }
  return ret;
}

result resource_manager::map(mapped_resource &mapped, surface_ptr const &surf, map_mode mm) {
  result ret = map_impl(mapped, surf, mm);
  if (ret == result::ok) {
    mapped_surf_ = surf;
  }
  return ret;
}

result resource_manager::unmap() {
  if (map_mode_ == map_mode_none) {
    return result::failed;
  }

  if (mapped_resource_.data != mapped_data_.data()) {
    switch (map_mode_) {
    case map_write:
    case map_write_discard:
      renderer_sync_(); // Sync for actual write.
      break;
    default:
      // do nothing.
      break;
    }
  }

  if (mapped_surf_) {
    mapped_surf_->unmap(mapped_resource_, map_mode_);
    mapped_surf_.reset();
  }
  if (mapped_buf_) {
    mapped_buf_->unmap(mapped_resource_, map_mode_);
    mapped_buf_.reset();
  }

  map_mode_ = map_mode_none;
  return result::ok;
}

void *resource_manager::reallocate_buffer(size_t sz) {
  assert(sz > 0);

  if (sz <= 0) {
    return nullptr;
  }

  if (sz > mapped_data_.size()) {
    mapped_data_.resize(sz);
  }

  return mapped_data_.data();
}

} // namespace salvia::resource