/********************************************************************
Copyright (C) 2007-2010 Ye Wu

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

purpose:	文件提供了无依赖库的网格模型的读、写、创建的声明

Modify Log:
		
*********************************************************************/

#ifndef SOFTARTX_MESH_IO_H
#define SOFTARTX_MESH_IO_H

#include "mesh.h"
#include "eflib/include/eflib.h"
#include <vector>

BEGIN_NS_SOFTARTX_RESOURCE()

h_mesh create_box(softart::renderer* psr);
h_mesh create_cylinder(softart::renderer* psr);
h_mesh create_planar(
					 softart::renderer* psr,
					 const eflib::vec3& start_pos,
					 const eflib::vec3& x_dir,	 const eflib::vec3& y_dir,
					 size_t repeat_x, size_t repeat_y,
					 bool positive_normal
					 );

END_NS_SOFTARTX_RESOURCE()

#endif //SOFTARTX_MESH_IO_H