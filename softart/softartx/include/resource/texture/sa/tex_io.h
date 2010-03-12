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

purpose:	文件声明了平台无关的纹理读、写、创建函数。

Modify Log:

*********************************************************************/

#ifndef SOFTARTX_TEX_IO_H
#define SOFTARTX_TEX_IO_H

#include "softartx/include/resource/resource_forward.h"
#include "softart/include/surface.h"
#include "softart/include/texture.h"
#include "softart/include/decl.h"
#include "eflib/include/math.h"

BEGIN_NS_SOFTARTX_RESOURCE()

class texture_io{
public:
	virtual softart::h_texture load(softart::renderer* pr, const std::_tstring& filename, softart::pixel_format tex_pxfmt) = 0;
	virtual softart::h_texture load_cube(softart::renderer* pr, const std::vector<std::_tstring>& cube_imgs, softart::pixel_format tex_pxfmt) = 0;

	virtual void save(const softart::surface& surf, const std::_tstring& filename, softart::pixel_format pxfmt) = 0;
};

END_NS_SOFTARTX_RESOURCE()

#endif //SOFTARTX_TEX_IO_H