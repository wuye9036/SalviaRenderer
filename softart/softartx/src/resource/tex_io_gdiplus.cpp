/*
Copyright (C) 2004-2005 Minmin Gong

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

#include "softartx/include/inc_gdiplus.h"

#include "softartx/include/resource/tex_io_gdiplus.h"

#include "softart/include/renderer.h"
#include "softart/include/resource_manager.h"

#include "eflib/include/eflib.h"

#include <FreeImage.h>

#include <tchar.h>


#include <algorithm>

using namespace Gdiplus;
using namespace efl;
using namespace std;

//h_texture load_texture_gdi(
//						   renderer_impl* psr,
//						   const std::wstring& filename,
//						   pixel_format fmt, size_t destWidth, size_t destHeight,
//						   const rect<int>& src
//						   )
//{
//	rect<int> src_copy(src);
//
//	Gdiplus::BitmapData srcbmpdata;
//	void* ptexdata = NULL;
//
//	Gdiplus::Bitmap file_bmp(filename.c_str());
//
//	//设置缺省
//	if((src.x | src.y | src.w | src.h) == 0){
//		src_copy.w = (int)file_bmp.GetWidth();
//		src_copy.h = (int)file_bmp.GetHeight();
//	}
//
//	if(destWidth == 0){destWidth = src_copy.w;}
//	if(destHeight == 0){destHeight = src_copy.h;}
//
//	//重置纹理
//	Gdiplus::Bitmap tex_src_bmp(destWidth, destHeight, PixelFormat32bppARGB);
//	Gdiplus::Graphics g(&tex_src_bmp);
//	tex.reset(destWidth, destHeight, fmt);
//
//	//渲染到等大的图上，并设置格式。
//	g.DrawImage(&file_bmp, Rect(0, 0, destWidth, destHeight), src_copy.x, src_copy.y, src_copy.w, src_copy.h, UnitPixel);
//
//	//锁定纹理与位图，拷贝数据
//	Rect rc(0, 0, destWidth, destHeight);
//	tex_src_bmp.LockBits(&rc, ImageLockModeRead, PixelFormat32bppARGB, &srcbmpdata);
//	tex.lock(&ptexdata, 0, rect<int>(0, 0, destWidth, destHeight), lock_write_only);
//	for(int iy = 0; iy < destHeight; ++iy){
//		pixel_format_convertor::convert_array(
//			fmt, 
//			pixel_format_color_bgra8, 
//			(byte*)ptexdata + destWidth*color_infos[fmt].size*iy,
//			(byte*)srcbmpdata.Scan0 + srcbmpdata.Stride * iy,
//			destWidth
//			);
//	}
//	tex.unlock();
//	tex_src_bmp.UnlockBits(&srcbmpdata);
//}