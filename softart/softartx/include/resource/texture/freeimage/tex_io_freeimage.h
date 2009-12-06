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

purpose:	文件声明了依赖于FreeImage库的纹理读、写、创建接口。

Modify Log:
		
*********************************************************************/

#ifndef SOFTARTX_TEX_IO_FREEIMAGE_H
#define SOFTARTX_TEX_IO_FREEIMAGE_H

#include "softartx/include/resource/texture/sa/tex_io.h"
#include "softart/include/colors.h"
#include "softart/include/decl.h"
#include "eflib/include/math.h"
#include <vector>

BEGIN_NS_SOFTARTX_RESOURCE()
#ifdef SOFTARTX_FREEIMAGE_ENABLED
class texture_io_fi: public texture_io{
public:
	virtual h_texture load(renderer* pr, const std::_tstring& filename, pixel_format tex_pxfmt);
	virtual h_texture load_cube(renderer* pr, const vector<_tstring>& filenames, pixel_format fmt);

	virtual void save(const surface& surf, const std::_tstring& filename, pixel_format pxfmt);

	static texture_to_fi& instance(){
		texture_to_fi ins;
		return ins;
	}
private:
	h_texture load( surface& surf, const rect<size_t>& dest_region, FIBITMAP* img, const rect<size_t>& src_region );
	h_texture load(renderer* pr, const FIBITMAP* img, const efl::rect<size_t>& src, pixel_format tex_pxfmt, size_t dest_width, size_t dest_height);

	texture_io_fi(){}
};
#endif
END_NS_SOFTARTX_RESOURCE()

#endif //SOFTARTX_TEX_IO_FREEIMAGE_H