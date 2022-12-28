#pragma once

#include <eflib/utility/shared_declaration.h>

namespace salvia::resource {
EFLIB_DECLARE_CLASS_SHARED_PTR(surface);
}

namespace salvia::core {
struct renderer_parameters;
EFLIB_DECLARE_CLASS_SHARED_PTR(renderer);
}

namespace salvia::ext{

EFLIB_DECLARE_CLASS_SHARED_PTR(swap_chain);
class swap_chain {
public:
  virtual salvia::resource::surface_ptr get_surface() = 0;
  virtual void present() = 0;
  virtual ~swap_chain() = default;
};

enum swap_chain_types {
  swap_chain_none = 0UL,
  swap_chain_default = 1UL,
  swap_chain_d3d11 = 2UL,
  swap_chain_gl = 3UL
};

enum renderer_types { renderer_none = 0UL, renderer_async = 1UL, renderer_sync = 2UL };

}

extern "C" {
void salviax_create_swap_chain_and_renderer(salvia::ext::swap_chain_ptr &out_swap_chain,
                                            salvia::core::renderer_ptr &out_renderer,
                                            salvia::core::renderer_parameters const *render_params,
                                            uint32_t renderer_type = salvia::ext::renderer_async,
                                            uint32_t swap_chain_type = salvia::ext::swap_chain_default);
}