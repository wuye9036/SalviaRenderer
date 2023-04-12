#include <salvia/ext/swap_chain/swap_chain_impl.h>

#include <salvia/core/async_renderer.h>
#include <salvia/core/renderer.h>
#include <salvia/core/sync_renderer.h>
#include <salvia/resource/texture.h>

#include <eflib/platform/dl_loader.h>

using namespace salvia::core;

namespace salvia::ext {

swap_chain_impl::swap_chain_impl(renderer_ptr const& renderer,
                                 renderer_parameters const& render_params) {
  renderer_ = renderer;

  surface_ = renderer_
                 ->create_tex2d(render_params.backbuffer_width,
                                render_params.backbuffer_height,
                                render_params.backbuffer_num_samples,
                                render_params.backbuffer_format)
                 ->subresource(0);

  if (render_params.backbuffer_num_samples > 1) {
    resolved_surface_ = renderer_
                            ->create_tex2d(render_params.backbuffer_width,
                                           render_params.backbuffer_height,
                                           1,
                                           render_params.backbuffer_format)
                            ->subresource(0);
  } else {
    resolved_surface_ = surface_;
  }
}

surface_ptr swap_chain_impl::get_surface() {
  return surface_;
}

void swap_chain_impl::present() {
  renderer_->flush();

  if (resolved_surface_ != surface_) {
    surface_->resolve(*resolved_surface_);
  }

  present_impl();
}

}  // namespace salvia::ext

extern "C" {
void salviax_create_swap_chain_and_renderer(salvia::ext::swap_chain_ptr& out_swap_chain,
                                            salvia::core::renderer_ptr& out_renderer,
                                            salvia::core::renderer_parameters const* render_params,
                                            uint32_t renderer_type,
                                            uint32_t swap_chain_type) {
  out_renderer.reset();
  out_swap_chain.reset();

  if (renderer_type == salvia::ext::renderer_sync) {
    out_renderer = create_sync_renderer();
  }

  if (renderer_type == salvia::ext::renderer_async) {
    out_renderer = create_async_renderer();
  }

  if (!out_renderer) {
    return;
  }

  if (swap_chain_type == salvia::ext::swap_chain_default) {
#if defined(SALVIA_EXT_D3D11_ENABLED)
    swap_chain_type = salvia::ext::swap_chain_d3d11;
#elif defined(SALVIA_EXT_GL_ENABLED)
    swap_chain_type = salvia::ext::swap_chain_gl;
#else
    return;
#endif
  }

  if (swap_chain_type == salvia::ext::swap_chain_gl) {
#if defined(SALVIA_EXT_GL_ENABLED)
    out_swap_chain = salvia::ext::create_gl_swap_chain(out_renderer, render_params);
#endif
  } else if (swap_chain_type == salvia::ext::swap_chain_d3d11) {
#if defined(SALVIA_EXT_D3D11_ENABLED)
    out_swap_chain = salvia::ext::create_d3d11_swap_chain(out_renderer, render_params);
#endif
  }
}
}