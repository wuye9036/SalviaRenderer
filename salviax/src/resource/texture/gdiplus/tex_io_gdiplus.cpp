/*
Copyright (C) 2007-2010 Minmin Gong, Ye Wu

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

#include "salviax/include/utility/inc_gdiplus.h"
#include "salviax/include/resource/texture/gdiplus/tex_io_gdiplus.h"
#include "salviar/include/renderer.h"
#include "salviar/include/resource_manager.h"
#include <tchar.h>
#include <algorithm>

using namespace Gdiplus;
using namespace eflib;
using namespace std;
using namespace salviar;
BEGIN_NS_SALVIAX_RESOURCE()
ULONG_PTR g_gdiplus = 0;
texture_io_gdiplus::texture_io_gdiplus()
{
	GdiplusStartupInput gdiplusStartupInput;

	GdiplusStartup(&g_gdiplus, &gdiplusStartupInput, NULL);

}
texture_io_gdiplus::~texture_io_gdiplus()
{
	GdiplusShutdown(g_gdiplus);
}
texture_io_gdiplus& texture_io_gdiplus::instance(){
	static texture_io_gdiplus s_instance;
	return s_instance;
}

/** Remarks:
     1. The size of dest region must be same as source region.
	 2. The pixel format of source image must be PixelFormat32bppARGB.
*/
bool texture_io_gdiplus::copy_image_to_surface(salviar::surface& surf, const rect<size_t>& dest_region, Bitmap* src_bmp, const rect<size_t>& src_region){
	if (src_bmp == NULL || dest_region.w != src_region.w || dest_region.h != src_region.h ){
		return false;
	}

	void* ptexdata = NULL;
	BitmapData bmp_data;

	Rect rc((INT)src_region.x, (INT)src_region.y, (INT)src_region.w, (INT)src_region.h);
	src_bmp->LockBits(&rc, ImageLockModeRead, PixelFormat32bppARGB, &bmp_data);
	surf.map(&ptexdata, map_write);
	for(size_t iy = 0; iy < src_region.h; ++iy){
		pixel_format_convertor::convert_array(
			surf.get_pixel_format(), 
			pixel_format_color_bgra8, 
			(byte*)ptexdata + ((dest_region.y + iy) * surf.get_width() + dest_region.x) * color_infos[surf.get_pixel_format()].size,
			(byte*)bmp_data.Scan0 + bmp_data.Stride * iy,
			(int)src_region.w
			);
	}
	surf.unmap();
	src_bmp->UnlockBits(&bmp_data);

	return true;
}

salviar::texture_ptr texture_io_gdiplus::load(salviar::renderer* pr, const std::_tstring& filename, salviar::pixel_format tex_pxfmt){
	Bitmap file_bmp(to_wide_string(filename).c_str());

	size_t src_w = (size_t)file_bmp.GetWidth();
	size_t src_h = (size_t)file_bmp.GetHeight();

	rect<size_t> region(0, 0, src_w, src_h);
	salviar::texture_ptr ret(pr->create_tex2d(src_w, src_h, 1, tex_pxfmt));
	load(*ret->get_surface(0), region, &file_bmp, region);
	return ret;
}

/**
Create cube texture with 6 images.
If images are not the same size, they would be stretched to the first image size.
*/
salviar::texture_ptr texture_io_gdiplus::load_cube(salviar::renderer *pr, const vector<_tstring> &filenames, salviar::pixel_format fmt){
	salviar::texture_ptr ret;
	rect<size_t> dest_region;

	if (filenames.size() != 6)
    {
		return ret;
	}

	for(int i_file = 0; i_file < 6; ++i_file)
    {
		Bitmap file_bmp( to_wide_string(filenames[i_file]).c_str() );
		rect<size_t> src_region(0, 0, file_bmp.GetWidth(), file_bmp.GetHeight());
		if (!ret)
        {
			dest_region = src_region;
			ret = pr->create_texcube( dest_region.w, dest_region.h, 1, fmt );
		}
		texture_cube* texcube = static_cast<texture_cube*>(ret.get());
		surface* cur_surf = texcube->get_face((cubemap_faces)i_file)->get_surface(0).get();
		load(*cur_surf, dest_region, &file_bmp, src_region);
	}

	return ret;
}

bool texture_io_gdiplus::load(salviar::surface& surf, const eflib::rect<size_t>& dest_region, Gdiplus::Bitmap* src_bmp, const eflib::rect<size_t>& src_region){
	if(src_bmp->GetPixelFormat() == PixelFormat32bppARGB && src_region.w == dest_region.w && src_region.h == src_region.h){
		return copy_image_to_surface( surf, dest_region, src_bmp, src_region);
	} else {
		Bitmap formatted_bmp((INT)dest_region.w, (INT)dest_region.h, PixelFormat32bppARGB);
		Graphics g(&formatted_bmp);
		Rect formatted_region = Rect(0, 0, (INT)dest_region.w, (INT)dest_region.h);
		Rect src_gdi_rect( efl_rect_to_gdiplus_rect<size_t>( src_region ) );
		g.DrawImage(
			src_bmp, formatted_region, 
			src_gdi_rect.X, src_gdi_rect.Y, src_gdi_rect.Width, src_gdi_rect.Height, 
			UnitPixel
			);
		rect<size_t> copy_src_rect( 0, 0, dest_region.w, dest_region.h);
		return copy_image_to_surface( surf, dest_region, &formatted_bmp, copy_src_rect);
	}
}

END_NS_SALVIAX_RESOURCE()
