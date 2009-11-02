/********************************************************************
Copyright (C) 2007-2008 Ye Wu

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

created:	2008/06/08
author:		Ye Wu

purpose:	GDI+ Device, Will Be Obseleted

Modify Log:
		
*********************************************************************/
#ifndef SOFTARTX_WINDOWSUTIL_H
#define SOFTARTX_WINDOWSUTIL_H

#include "softartx/include/inc_gdiplus.h"

#include "softart/include/framebuffer.h"
#include "eflib/include/platform.h"

#include <tchar.h>
#include <boost/smart_ptr.hpp>
#include <algorithm>
#include <stdio.h>

//使用GDI+对程序界面进行绘制
class WindowsUtil
{
public:
	virtual void UpdateBackBuffer(const framebuffer* fb) = 0;

	virtual void Render(Gdiplus::Graphics* pgraph, const Gdiplus::Rect& rc) = 0;
	virtual void Render(HDC hdc, const RECT& rc) = 0;

	WindowsUtil(void);
	virtual ~WindowsUtil(void);

private:
	virtual void SetBufferSize(size_t width, size_t height) = 0;
};

class WindowsUtil_GDIPlus : public WindowsUtil
{
	framebuffer* pfb_;

public:
	virtual void UpdateBackBuffer(const framebuffer* fb);

	virtual void Render(Gdiplus::Graphics* pgraph, const Gdiplus::Rect& rc);
	virtual void Render(HDC hdc, const RECT& rc);

private:
	virtual void SetBufferSize(size_t width, size_t height);
	boost::shared_ptr<Gdiplus::Bitmap> pBitmap_;
};

class GdiplusInitializer
{
	Gdiplus::GdiplusStartupInput gdiPlusInput_;
	ULONG_PTR gdiPlusToken_;

	static GdiplusInitializer init_;
public:
	GdiplusInitializer()
	{
#ifdef _CONSOLE
		_tprintf(_T("%s"), _T("Gdiplus Initialized.\n"));
#else
		Gdiplus::GdiplusStartup(&gdiPlusToken_, &gdiPlusInput_, NULL);
#endif
	}
	~GdiplusInitializer()
	{
#ifdef _CONSOLE
		_tprintf(_T("%s"), _T("Gdiplus Shutdown.\n"));
#else
		Gdiplus::GdiplusShutdown(gdiPlusToken_);
#endif
	}
};

#endif //SOFTARTX_WINDOWSUTIL_H