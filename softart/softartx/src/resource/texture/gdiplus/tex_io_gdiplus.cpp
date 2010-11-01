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

#include "eflib/include/eflib.h"
#include "softartx/include/utility/inc_gdiplus.h"
#include "softartx/include/resource/texture/gdiplus/tex_io_gdiplus.h"
#include "softart/include/renderer.h"
#include "softart/include/resource_manager.h"
#include <tchar.h>
#include <algorithm>

using namespace Gdiplus;
using namespace eflib;
using namespace std;
using namespace softart;
BEGIN_NS_SOFTARTX_RESOURCE()
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

//将指定的GDI位图部分拷贝到Surface的指定区域。目标区域需要与源区域大小相同，且源位图格式为PixelFormat32bppARGB格式。
bool texture_io_gdiplus::copy_image_to_surface(softart::surface& surf, const rect<size_t>& dest_region, Bitmap* src_bmp, const rect<size_t>& src_region){
	if (src_bmp == NULL || dest_region.w != src_region.w || dest_region.h != src_region.h ){
		return false;
	}

	//锁定纹理与位图，拷贝数据
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

softart::h_texture texture_io_gdiplus::load(softart::renderer* pr, const std::_tstring& filename, softart::pixel_format tex_pxfmt){
	Bitmap file_bmp(to_wide_string(filename).c_str());

	size_t src_w = (size_t)file_bmp.GetWidth();
	size_t src_h = (size_t)file_bmp.GetHeight();

	rect<size_t> region(0, 0, src_w, src_h);
	softart::h_texture ret(pr->create_tex2d(src_w, src_h, 1, tex_pxfmt));
	load( ret->get_surface(0), region, &file_bmp, region);
	return ret;
}

//使用六张图像创建Cube纹理。Cube纹理的每面大小和第一张纹理的大小相同。如果其他文件的大小与第一张不同，则按第一张的大小缩放。
softart::h_texture texture_io_gdiplus::load_cube(softart::renderer *pr, const vector<_tstring> &filenames, softart::pixel_format fmt){
	softart::h_texture ret;
	rect<size_t> dest_region;

	if (filenames.size() != 6){
		return ret;
	}

	for(int i_file = 0; i_file < 6; ++i_file){
		Bitmap file_bmp( to_wide_string(filenames[i_file]).c_str() );
		rect<size_t> src_region(0, 0, file_bmp.GetWidth(), file_bmp.GetHeight());
		if (!ret){
			dest_region = src_region;
			ret = pr->create_texcube( dest_region.w, dest_region.h, 1, fmt );
		}
		texture_cube* texcube = static_cast<texture_cube*>(ret.get());
		softart::surface& cur_surf = texcube->get_face((cubemap_faces)i_file).get_surface(0);
		load(cur_surf, dest_region, &file_bmp, src_region);
	}

	return ret;
}

bool texture_io_gdiplus::load(softart::surface& surf, const eflib::rect<size_t>& dest_region, Gdiplus::Bitmap* src_bmp, const eflib::rect<size_t>& src_region){
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

END_NS_SOFTARTX_RESOURCE()
