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

purpose:	文件声明了依赖于GDIPlus的文件读、写、创建函数。

Modify Log:
		
*********************************************************************/

#ifndef SOFTARTX_TEX_IO_GDIPLUS_H
#define SOFTARTX_TEX_IO_GDIPLUS_H

#include "softart/include/surface.h"
#include "softart/include/decl.h"

#include "eflib/include/math.h"

#include <string>
#include <vector>

h_texture create_texture_gdip(
					   renderer_impl* psr,
					   const std::wstring& filename,
					   pixel_format fmt, size_t destWidth = 0, size_t destHeight = 0, 
					   const efl::rect<int>& src = efl::rect<int>(0, 0, 0, 0)
					   );

#endif //SOFTARTX_TEX_IO_GDIPLUS_H