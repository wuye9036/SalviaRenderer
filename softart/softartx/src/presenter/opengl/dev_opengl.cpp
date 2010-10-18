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

#include "softartx/include/presenter/opengl/dev_opengl.h"
#include "softart/include/framebuffer.h"
#include "softart/include/surface.h"
#include <tchar.h>

using namespace efl;
using namespace softart;

BEGIN_NS_SOFTARTX_PRESENTER()

dev_opengl::dev_opengl(HWND hwnd): hwnd_(hwnd), width_(0), height_(0){
	init_device();
}

dev_opengl::~dev_opengl(){
	glDeleteTextures(1, &buftex_);
}

void dev_opengl::init_device()
{
	hdc_ = GetDC(hwnd_);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize		= sizeof(pfd);
	pfd.nVersion	= 1;
	pfd.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType	= PFD_TYPE_RGBA;
	pfd.cColorBits	= static_cast<BYTE>(32);
	pfd.cDepthBits	= static_cast<BYTE>(0);
	pfd.cStencilBits = static_cast<BYTE>(0);
	pfd.iLayerType	= PFD_MAIN_PLANE;

	int pixelFormat = ::ChoosePixelFormat(hdc_, &pfd);
	BOOST_ASSERT(pixelFormat != 0);

	::SetPixelFormat(hdc_, pixelFormat, &pfd);
	::DescribePixelFormat(hdc_, pixelFormat, sizeof(pfd), &pfd);

	hrc_ = ::wglCreateContext(hdc_);
	::wglMakeCurrent(hdc_, hrc_);

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &buftex_);
}

h_dev_opengl dev_opengl::create_device(HWND hwnd){
	return h_dev_opengl(new dev_opengl( hwnd ));
}

//inherited
void dev_opengl::present(const softart::surface& surf)
{
	byte* src_addr = NULL;
	surf.map((void**)(&src_addr), map_read);
	if( src_addr == NULL ) return;

	std::vector<byte> dest(surf.get_width() * surf.get_height() * 4);
	for(size_t irow = 0; irow < surf.get_height(); ++irow)
	{
		byte* dest_addr = &dest[irow * surf.get_width() * 4];
		pixel_format_convertor::convert_array(
			pixel_format_color_rgba8, surf.get_pixel_format(),
			dest_addr, src_addr,
			int(surf.get_width())
			);
		src_addr += surf.get_pitch();
	}

	uint32_t surf_width = static_cast<uint32_t>(surf.get_width());
	uint32_t surf_height = static_cast<uint32_t>(surf.get_height());

	glViewport(0, 0, surf_width, surf_height);

	glBindTexture(GL_TEXTURE_2D, buftex_);
	if ((width_ < surf_width) || (height_ < surf_height))
	{
		width_ = surf_width;
		height_ = surf_height;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, &dest[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surf_width, surf_height, GL_RGBA, GL_UNSIGNED_BYTE, &dest[0]);
	}

	surf.unmap();

	float fw = static_cast<float>(surf_width) / width_;
	float fh = static_cast<float>(surf_height) / height_;

	glBegin(GL_TRIANGLE_STRIP);

	glTexCoord2f(0, 0);
	glVertex3f(-1, +1, 0);

	glTexCoord2f(fw, 0);
	glVertex3f(+1, +1, 0);

	glTexCoord2f(0, fh);
	glVertex3f(-1, -1, 0);

	glTexCoord2f(fw, fh);
	glVertex3f(+1, -1, 0);

	glEnd();

	::SwapBuffers(hdc_);
}

END_NS_SOFTARTX_PRESENTER()

void softart_create_presenter_device(softart::h_device& dev, void* param)
{
	dev = softartx::presenter::dev_opengl::create_device(static_cast<HWND>(param));
}

