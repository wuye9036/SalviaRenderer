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

purpose:	该文件提供了与D3D9有关的模型的构造与读取、写入的接口声明。

Modify Log:
		
*********************************************************************/

#ifndef SOFTARTX_MESH_IO_D3D9_H
#define SOFTARTX_MESH_IO_D3D9_H

#include "softartx/include/mesh.h"

#include "softart/include/decl.h"

#include "eflib/include/stl_utilities.h"

struct ID3DXMesh;
typedef ID3DXMesh* LPD3DXMESH;

h_mesh create_mesh_from_dx9mesh(renderer_impl* psr, LPD3DXMESH pmesh);
h_mesh create_mesh_from_xfile(renderer_impl* psr, const _tstring& filename);

#endif //SOFTARTX_MESH_IO_D3D9_H
