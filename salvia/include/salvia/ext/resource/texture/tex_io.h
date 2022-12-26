#pragma once

#include <salviax/include/resource/resource_forward.h>
#include <salvia/resource/colors.h>
#include <salviar/include/decl.h>
#include <eflib/string/string.h>
#include <eflib/math/math.h>

#include <vector>

struct FIBITMAP;

namespace salviax::resource{

salviar::texture_ptr	load_texture(
	salviar::renderer* rend,
	const std::_tstring& filename, salviar::pixel_format tex_format
	);

salviar::texture_ptr	load_cube(
	salviar::renderer* rend,
	const std::vector<std::_tstring>& filenames, salviar::pixel_format tex_format
	);

void					save_surface(
	salviar::renderer* rend,
	salviar::surface_ptr const& surf, std::_tstring const& filename, salviar::pixel_format image_format
	);

}
