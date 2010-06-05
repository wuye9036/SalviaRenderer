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

#include "softartx/include/presenter/gdiplus/dev_gdiplus.h"
#include "softart/include/surface.h"

using namespace boost;
using namespace std;
using namespace Gdiplus;
using namespace efl;
using namespace softart;

BEGIN_NS_SOFTARTX_PRESENTER()

gdiplus_initializer gdiplus_initer;


dev_gdiplus::dev_gdiplus(HWND hwnd): hwnd_(hwnd){
}

dev_gdiplus::~dev_gdiplus(){
}

h_dev_gdiplus dev_gdiplus::create_device(HWND hwnd){
	return h_dev_gdiplus( new dev_gdiplus( hwnd ) );
}

void dev_gdiplus::present(const softart::surface& surf){
	if ( !pbmp_ || 
		pbmp_->GetWidth() < (INT)surf.get_width() || 
		pbmp_->GetHeight() < (INT)surf.get_height() )
	{
		pbmp_.reset( new Bitmap( (INT)surf.get_width(), (INT)surf.get_height(), PixelFormat32bppRGB) );
	}

	Gdiplus::Graphics g(::GetDC(hwnd_));

	//将framebuffer的surface拷贝到bitmap中
	Rect rcFramebuffer(0, 0, (INT)surf.get_width(), (INT)surf.get_height());

	void* pfbdata = NULL;
	BitmapData bmpData;

	surf.map(&pfbdata, map_read);
	pbmp_->LockBits(&rcFramebuffer, ImageLockModeWrite, PixelFormat32bppRGB, &bmpData);

	void* pixels = (void*)bmpData.Scan0;
	
	for(size_t iheight = 0; iheight < surf.get_height(); ++iheight)
	{
		softart::pixel_format rt_pxfmt = surf.get_pixel_format();
		byte* surface_scanline_addr = (byte*)pfbdata + iheight*get_color_info(rt_pxfmt).size*surf.get_width();
		byte* bmp_scanline_addr = ((uint8_t*)pixels) + bmpData.Stride * iheight;

		pixel_format_convertor::convert_array(
			pixel_format_color_bgra8, rt_pxfmt, 
			bmp_scanline_addr, surface_scanline_addr,
			int(surf.get_width())
			);
	}

	surf.unmap();
	pbmp_->UnlockBits(&bmpData);

	//渲染到设备上
	g.DrawImage(pbmp_.get(),
		Rect(0, 0, static_cast<UINT>(surf.get_width()), static_cast<UINT>(surf.get_height())));
}

END_NS_SOFTARTX_PRESENTER()

void softart_create_presenter_device(softart::h_device& dev, void* param)
{
	dev = softartx::presenter::dev_gdiplus::create_device(static_cast<HWND>(param));
}

