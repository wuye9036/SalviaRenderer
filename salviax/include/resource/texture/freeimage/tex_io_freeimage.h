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

#ifndef SALVIAX_TEX_IO_FREEIMAGE_H
#define SALVIAX_TEX_IO_FREEIMAGE_H

#include <salviax/include/utility/user_config.h>

#include <salviax/include/resource/texture/sa/tex_io.h>
#include <salviar/include/colors.h>
#include <salviar/include/decl.h>
#include <eflib/include/math/math.h>
#include <vector>

struct FIBITMAP;

BEGIN_NS_SALVIAX_RESOURCE()
#ifdef SALVIAX_FREEIMAGE_ENABLED
class texture_io_fi: public texture_io{
public:
	virtual salviar::h_texture load(salviar::renderer* pr, const std::_tstring& filename, salviar::pixel_format tex_pxfmt);
	virtual salviar::h_texture load_cube(salviar::renderer* pr, const std::vector<std::_tstring>& filenames, salviar::pixel_format fmt);

	virtual void save(const salviar::surface& surf, const std::_tstring& filename, salviar::pixel_format pxfmt);

	static texture_io_fi& instance(){
		static texture_io_fi ins;
		return ins;
	}
private:
	bool load( salviar::surface& surf, const eflib::rect<size_t>& dest_region, FIBITMAP* img, const eflib::rect<size_t>& src_region );
	salviar::h_texture load(salviar::renderer* pr, FIBITMAP* img, const eflib::rect<size_t>& src, salviar::pixel_format tex_pxfmt, size_t dest_width, size_t dest_height);

	texture_io_fi(){}
};
#endif
END_NS_SALVIAX_RESOURCE()

#endif //SALVIAX_TEX_IO_FREEIMAGE_H