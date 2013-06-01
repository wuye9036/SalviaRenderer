/********************************************************************
Copyright (C) 2007-2012 Ye Wu

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

purpose:	Support built-in meshes without any dependency.

Modify Log:
		
*********************************************************************/

#ifndef SALVIAX_MESH_IO_H
#define SALVIAX_MESH_IO_H

#include <salviax/include/resource/mesh/sa/mesh.h>
#include <eflib/include/math/vector.h>
#include <vector>

BEGIN_NS_SALVIAX_RESOURCE();

mesh_ptr create_box(salviar::renderer* psr);
mesh_ptr create_cylinder(salviar::renderer* psr);
mesh_ptr create_planar(
					 salviar::renderer* psr,
					 const eflib::vec3& start_pos,
					 const eflib::vec3& x_dir,	 const eflib::vec3& y_dir,
					 size_t repeat_x, size_t repeat_y,
					 bool positive_normal
					 );
mesh_ptr create_cone(
	salviar::renderer* psr,
	eflib::vec3 const& bottom_center,
	float radius, eflib::vec3 const& up_dir, int segments
	);

END_NS_SALVIAX_RESOURCE();

#endif //SALVIAX_MESH_IO_H