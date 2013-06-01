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

#ifndef SALVIAX_DEV_OPENGL_H
#define SALVIAX_DEV_OPENGL_H

#include <eflib/include/platform/config.h>

#include <salviax/include/presenter/presenter_forward.h>
#include <salviar/include/presenter_dev.h>
#include <eflib/include/math/math.h>
#include <eflib/include/utility/shared_declaration.h>

#include <windows.h>
#include <GL/GL.h>

BEGIN_NS_SALVIAX_PRESENTER();

EFLIB_DECLARE_CLASS_SHARED_PTR(dev_opengl);

class dev_opengl : public salviar::device
{
public:
	static dev_opengl_ptr create_device(HWND hwnd);

	//inherited
	virtual void present(const salviar::surface& surf);

	~dev_opengl();

private:
	dev_opengl(HWND hwnd);
	void init_device();

	HWND hwnd_;
	HDC hdc_;
	HGLRC hrc_;
	GLuint buftex_;

	uint32_t width_, height_;
};

END_NS_SALVIAX_PRESENTER();

#ifdef salviax_opengl_presenter_EXPORTS
	#define SALVIAX_API __declspec(dllexport)
#else
	#define SALVIAX_API __declspec(dllimport)
#endif

extern "C"
{
	SALVIAX_API void salviax_create_presenter_device(salviar::device_ptr& dev, void* param);
}

#endif //SALVIAX_DEV_OPENGL_H
