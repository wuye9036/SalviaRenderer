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


dev_gdiplus::dev_gdiplus(HWND hwnd): hwnd_(hwnd), fb_(NULL){
}

dev_gdiplus::~dev_gdiplus(){
}

h_dev_gdiplus dev_gdiplus::create_device(HWND hwnd){
	return h_dev_gdiplus( new dev_gdiplus( hwnd ) );
}

void dev_gdiplus::attach_framebuffer(softart::framebuffer *pfb){
	if (fb_ == pfb) {
		return;
	}
	fb_ = pfb;

	rc_ = Rect(0, 0, fb_->get_width(), fb_->get_height());

	if ( !pbmp_ || 
		pbmp_->GetWidth() < (INT)fb_->get_width() || 
		pbmp_->GetHeight() < (INT)fb_->get_height() )
	{
		pbmp_.reset( new Bitmap( (INT)fb_->get_width(), (INT)fb_->get_height(), PixelFormat32bppRGB) );
	}
}

void dev_gdiplus::present(){
	if (fb_ == NULL){
		return;
	}

	Gdiplus::Graphics g(::GetDC(hwnd_));

	//将framebuffer的surface拷贝到bitmap中
	Rect rcFramebuffer(0, 0, (INT)fb_->get_width(), (INT)fb_->get_height());
	softart::surface* rt = fb_->get_render_target(render_target_color, 0);

	void* pfbdata = NULL;
	BitmapData bmpData;

	rt->map(&pfbdata, map_read);
	pbmp_->LockBits(&rcFramebuffer, ImageLockModeWrite, PixelFormat32bppRGB, &bmpData);

	void* pixels = (void*)bmpData.Scan0;
	
	for(size_t iheight = 0; iheight < fb_->get_height(); ++iheight)
	{
		softart::pixel_format rt_pxfmt = rt->get_pixel_format();
		byte* surface_scanline_addr = (byte*)pfbdata + (fb_->get_height() - iheight - 1)*get_color_info(rt_pxfmt).size*rt->get_width();
		byte* bmp_scanline_addr = ((uint8_t*)pixels) + bmpData.Stride * iheight;

		pixel_format_convertor::convert_array(
			pixel_format_color_bgra8, rt_pxfmt, 
			bmp_scanline_addr, surface_scanline_addr,
			int(fb_->get_width())
			);
	}

	rt->unmap();
	pbmp_->UnlockBits(&bmpData);

	//渲染到设备上
	g.DrawImage(pbmp_.get(), rc_);
}

END_NS_SOFTARTX_PRESENTER()

void create_device(softart::h_device& dev, void* param)
{
	dev = softartx::presenter::dev_gdiplus::create_device(static_cast<HWND>(param));
}

