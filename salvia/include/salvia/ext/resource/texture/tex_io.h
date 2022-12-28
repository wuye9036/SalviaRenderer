#pragma once

#include <eflib/math/math.h>

#include <salvia/common/colors.h>
#include <salvia/common/colors_convertors.h>

#include <salvia/core/decl.h>

#include <vector>

struct FIBITMAP;

namespace salvia::resource{
using surface_ptr = std::shared_ptr<class surface>;
}

namespace salvia::ext::resource {

salvia::resource::texture_ptr load_texture(salvia::core::renderer *rend, const std::string &filename,
                                  salvia::pixel_format tex_format);

salvia::resource::texture_ptr load_cube(salvia::core::renderer *rend, const std::vector<std::string> &filenames,
                               salvia::pixel_format tex_format);

void save_surface(salvia::core::renderer *rend, salvia::resource::surface_ptr const &surf,
                  std::string const &filename, salvia::pixel_format image_format);

} // namespace salvia::ext::resource
