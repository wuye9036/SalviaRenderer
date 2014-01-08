#include <salviax/include/resource/texture/tex_io.h>

#include <salviax/include/utility/freeimage_utilities.h>
#include <salviar/include/renderer.h>
#include <salviar/include/surface.h>
#include <salviar/include/texture.h>
#include <salviar/include/mapped_resource.h>
#include <FreeImage.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/static_assert.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>
#include <memory>

using namespace eflib;
using namespace std;
using namespace salviar;
using namespace salviax::utility;

BEGIN_NS_SALVIAX_RESOURCE();

// Copy pixels FIBITMAP to surface as following steps
//	*> Get color component informations from FIBITMAP
//	*> Convert color from FIBITMAP to immediate RGBA color which is supported by SALVIA.
//	*> Convert immediate color to final surface format.
//
// Template parameters:
//	*> FIColorT: The format of color of FIBITMAP
template<typename FIColorT>
bool copy_image_to_surface_impl(surface_ptr const& surf, FIBITMAP* image,
	typename FIUC<FIColorT>::CompT default_alpha = (typename FIUC<FIColorT>::CompT)(0) )
{
	if (image == NULL)
	{
		return false;
	}

	size_t		 image_pitch = FreeImage_GetPitch(image);
	size_t		 image_bpp = (FreeImage_GetBPP(image) >> 3);
	pixel_format surface_format = surf->get_pixel_format();
	pixel_format inter_format = salvia_rgba_color_type<FIColorT>::fmt;
	BYTE*		 source_line = FreeImage_GetBits(image);

	for(size_t y = 0; y < surf->get_height(); ++y)
	{
		byte* src_pixel = source_line;
		for(size_t x = 0; x < surf->get_width(); ++x)
		{
			FIUC<FIColorT> uc((typename FIUC<FIColorT>::CompT*)src_pixel, default_alpha);
			typename salvia_rgba_color_type<FIColorT>::type c(uc.r, uc.g, uc.b, uc.a);
			pixel_format_convertor::convert(surface_format, inter_format, surf->texel_address(x, y, 0), &c);
			src_pixel += image_bpp;
		}
		source_line += image_pitch;
	}

	return true;
}

// Copy region of image to dest region of surface.
// If the size of source and destination are different, it will be stretch copy with bi-linear interpolation.
bool copy_image_to_surface(surface_ptr const& surf, FIBITMAP* img)
{
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType( img );

	if(image_type == FIT_RGBAF)
	{
		return copy_image_to_surface_impl<FIRGBAF>(surf, img);
	}
	if(image_type == FIT_BITMAP)
	{
		if(FreeImage_GetColorType(img) == FIC_RGBALPHA)
		{
			return copy_image_to_surface_impl<RGBQUAD>(surf, img);
		}
		else
		{
			return copy_image_to_surface_impl<RGBTRIPLE>(surf, img);
		}
	}

	return false;
}

// Load image file to new texture
texture_ptr load_texture(renderer* rend, const std::_tstring& filename, pixel_format tex_format)
{
	FIBITMAP* img = load_image(filename);
	texture_ptr ret;

	size_t src_w = FreeImage_GetWidth(img);
	size_t src_h = FreeImage_GetHeight(img);

	ret = rend->create_tex2d(src_w, src_h, 1, tex_format);

	if( !copy_image_to_surface(ret->get_surface(0), img) )
	{
		ret.reset();
	}

	FreeImage_Unload(img);

	return ret;
}


// Create cube texture by six images.
// Size of first texture is the size of cube face.
// If other textures are not same size as first, just stretch it.
texture_ptr load_cube(renderer* rend, const vector<_tstring>& filenames, pixel_format tex_format)
{
	texture_ptr ret;

	auto image_deleter  = [](FIBITMAP* bmp){ FreeImage_Unload(bmp); };
	size_t tex_width  = 0;
	size_t tex_height = 0;

	for(int i_cubeface = 0; i_cubeface < 6; ++i_cubeface)
	{
		std::unique_ptr<FIBITMAP, decltype(image_deleter)>
			cube_img(load_image(filenames[i_cubeface]), image_deleter);

		if (cube_img.get() == nullptr)
		{
			ret.reset();
			return ret;
		}

		size_t img_w = FreeImage_GetWidth (cube_img.get());
		size_t img_h = FreeImage_GetHeight(cube_img.get());

		// Create texture cube while first face is created.
		if ( !ret )
		{
			tex_width  = img_w;
			tex_height = img_h;
			ret = rend->create_texcube(img_w, img_h, 1, tex_format);
		}
		else
		{
			if (tex_width != img_w || tex_height != img_h)
			{
				ret.reset();
				return ret;
			}
		}

		texture_cube* cube_tex = static_cast<texture_cube*>(ret.get());
		surface_ptr face_surface = cube_tex->get_face( cubemap_faces(i_cubeface) )->get_surface(0);
		copy_image_to_surface( face_surface, cube_img.get() );
	}

	return ret;
}

// Save surface as PNG or HRD formatted file.
void save_surface(renderer* rend, surface_ptr const& surf, _tstring const& filename, pixel_format image_format)
{
	FREE_IMAGE_TYPE fit = FIT_UNKNOWN;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	FIBITMAP* image = NULL;
	int surface_width  = static_cast<int>( surf->get_width() );
	int surface_height = static_cast<int>( surf->get_height() );

	switch(image_format)
	{
	case pixel_format_color_bgra8:
		fit = FIT_BITMAP;
		fif = FIF_PNG;
		image = FreeImage_AllocateT(fit, surface_width, surface_height, 32, 0x0000FF, 0x00FF00, 0xFF0000);
		break;
	case pixel_format_color_rgb32f:
		fit = FIT_RGBF;
		fif = FIF_HDR;
		image = FreeImage_AllocateT(fit, surface_width, surface_height, 96);
		break;
	default:
		EFLIB_ASSERT(false, "Unsupport format was used£¡");
		return;
	}

	mapped_resource mapped;
	rend->map(mapped, surf, map_read);

	byte* 		 surf_data = reinterpret_cast<byte*>(mapped.data);
	byte*		 img_data = FreeImage_GetBits(image);
	pixel_format surf_format = surf->get_pixel_format();
	size_t		 height = surf->get_height();
	size_t		 width = surf->get_width();

	for(size_t y = 0; y < height; ++y)
	{
		pixel_format_convertor::convert_array(
			image_format, surf_format,
			img_data, surf_data, int(width)
			);
		surf_data +=  mapped.row_pitch;
		img_data += FreeImage_GetPitch(image);
	}

	rend->unmap();

	FreeImage_Save(fif, image, to_ansi_string(filename).c_str());
	FreeImage_Unload(image);
}

END_NS_SALVIAX_RESOURCE();

/*
Copyright (C) 2007-2012 Minmin Gong, Ye Wu

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
