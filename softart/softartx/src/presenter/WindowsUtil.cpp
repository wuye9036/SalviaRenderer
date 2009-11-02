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

#include "softartx/include/presenter/WindowsUtil.h"
#include "softart/include/surface.h"

using namespace boost;
using namespace std;
using namespace Gdiplus;
using namespace efl;

GdiplusInitializer GdiplusInitializer::init_;

WindowsUtil::WindowsUtil(void)
{
}

WindowsUtil::~WindowsUtil(void)
{
}

//更新后备缓冲
void WindowsUtil_GDIPlus::UpdateBackBuffer(const framebuffer* fb)
{
	SetBufferSize(fb->get_width(), fb->get_height());

	Rect rcFramebuffer(0, 0, (INT)fb->get_width(), (INT)fb->get_height());
	surface* rt = const_cast<framebuffer*>(fb)->get_render_target(render_target_color, 0);

	//修改bitmap
	void* pfbdata = NULL;
	BitmapData bmpData;

	rt->lock(&pfbdata, rect<size_t>(0, 0, fb->get_width(), fb->get_height()), lock_read_only);
	pBitmap_->LockBits(&rcFramebuffer, ImageLockModeWrite, PixelFormat32bppRGB, &bmpData);

	void* pixels = (void*)bmpData.Scan0;
	
	for(size_t iheight = 0; iheight < fb->get_height(); ++iheight)
	{
		uint8_t* scanline_addr = ((uint8_t*)pixels) + bmpData.Stride * iheight;
		uint32_t* pixels_line = (uint32_t*)scanline_addr;

		pixel_format_convertor::convert_array(pixel_format_color_bgra8, 
			rt->get_pixel_format(), 
			pixels_line, (byte*)pfbdata + (fb->get_height() - iheight - 1)*color_infos[rt->get_pixel_format()].size*rt->get_width(), int(fb->get_width()));
	}

	rt->unlock();
	pBitmap_->UnlockBits(&bmpData);
}

//调整后备缓冲大小
void WindowsUtil_GDIPlus::SetBufferSize(size_t width, size_t height)
{
	//如果现有的bitmap空间足够，那么就仍然使用以前的bitmap。
	pBitmap_.reset(new Bitmap(width, height, PixelFormat32bppARGB));
}

void WindowsUtil_GDIPlus::Render(Graphics* pgraph, const Rect& /*rc*/)
{
	//渲染到设备上
	pgraph->DrawImage(pBitmap_.get(), 0.0f, 0.0f);
}

void WindowsUtil_GDIPlus::Render(HDC hdc, const RECT& rc)
{
	Graphics g(hdc);
	Rect r(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
	Render(&g, r);
}