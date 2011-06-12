/*
Copyright (C) 2007-2010 Minmin Gong, Ye Wu

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

#ifndef SALVIAX_D3D9_UTILITIES_H
#define SALVIAX_D3D9_UTILITIES_H

#include "utility_forward.h"
#include "inc_d3d9.h"
#include "salviar/include/handles.h"

BEGIN_NS_SALVIAX_UTILITY()

class d3d9_device;
DECL_HANDLE( d3d9_device, h_d3d9_device );

struct d3d9_device_param{
	UINT adapter;
	D3DDEVTYPE devtype;
	HWND focuswnd;
	DWORD behavior;
};

class d3d9_device{
private:
	IDirect3D9* pd3d9_;
	IDirect3DDevice9* pd3ddev9_;

	d3d9_device(const d3d9_device_param& param, D3DPRESENT_PARAMETERS& present_params);
public:
	static h_d3d9_device create(const d3d9_device_param& param, D3DPRESENT_PARAMETERS& present_params);
	IDirect3D9* get_d3d9() const;
	IDirect3DDevice9* get_d3d_device9() const;
	~d3d9_device();
};

END_NS_SALVIAX_UTILITY()

#endif // SALVIAX_D3D9_UTILITIES_H
