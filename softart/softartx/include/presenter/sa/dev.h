/********************************************************************
Copyright (C) 2007-2010 Ye Wu, Minmin Gong

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

purpose:	该文件提供了平台无关的显示设备的接口。通过显示设备可以将软件渲染器的后备缓冲渲染到屏幕上。

Modify Log:
		
*********************************************************************/

#ifndef SOFTARTX_DEV_H
#define SOFTARTX_DEV_H

#include "softart/include/dev.h"

#define BEGIN_NS_SOFTARTX_PRESENTER() namespace softartx{ namespace presenter{
#define END_NS_SOFTARTX_PRESENTER() }}

//BEGIN_NS_SOFTARTX_PRESENTER()
//class device{
//public:
//	virtual void attach_framebuffer(framebuffer* pfb) = 0;
//	virtual void present() = 0;
//
//	virtual ~device(){}
//};
//END_NS_SOFTARTX_PRESENTER()

#endif //SOFTARTX_DEV_H