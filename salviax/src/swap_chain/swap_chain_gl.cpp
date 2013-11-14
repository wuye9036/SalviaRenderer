#if defined(SALVIAX_GL_ENABLED)

#include <salviax/include/swap_chain/swap_chain_impl.h>

#include <salviar/include/surface.h>
#include <salviar/include/renderer.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

#include <windows.h>
#include <GL/GL.h>

#pragma comment(lib, "opengl32.lib")

using namespace salviar;

BEGIN_NS_SALVIAX();

class gl_swap_chain: public swap_chain_impl
{
public:
	gl_swap_chain(
		renderer_ptr const& renderer,
		renderer_parameters const& params)
		: swap_chain_impl(renderer, params)
		, window_(nullptr)
		, dc_(nullptr), glrc_(nullptr)
		, tex_(0)
		, width_(0), height_(0)
	{
		window_ = reinterpret_cast<HWND>(params.native_window);
		initialize();
	}

	~gl_swap_chain()
	{
		glDeleteTextures(1, &tex_);
	}

	void initialize()
	{
		dc_ = GetDC(window_);

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

		int pixelFormat = ::ChoosePixelFormat(dc_, &pfd);
		assert(pixelFormat != 0);

		::SetPixelFormat(dc_, pixelFormat, &pfd);
		::DescribePixelFormat(dc_, pixelFormat, sizeof(pfd), &pfd);

		glrc_ = ::wglCreateContext(dc_);
		::wglMakeCurrent(dc_, glrc_);

		{
			std::string ext_str(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
			if (ext_str.find("WGL_EXT_swap_control") != std::string::npos)
			{
				typedef BOOL (APIENTRY *wglSwapIntervalEXTFUNC)(int interval);
				wglSwapIntervalEXTFUNC wglSwapIntervalEXT = reinterpret_cast<wglSwapIntervalEXTFUNC>((void*)(::wglGetProcAddress("wglSwapIntervalEXT")));
				wglSwapIntervalEXT(0);
			}
		}

		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &tex_);
	}

	void present_impl()
	{
		size_t surface_width  = resolved_surface_->get_width();
		size_t surface_height = resolved_surface_->get_height();

		glViewport(
			0, 0,
			static_cast<uint32_t>(surface_width),
			static_cast<uint32_t>(surface_height)
			);

		byte* src_addr = NULL;
		resolved_surface_->map((void**)(&src_addr), map_read);
		if(src_addr == NULL) return;

		std::vector<byte> dest(resolved_surface_->get_width() * resolved_surface_->get_height() * 4);
		for(size_t irow = 0; irow < resolved_surface_->get_height(); ++irow)
		{
			byte* dest_addr = &dest[irow * resolved_surface_->get_width() * 4];
			pixel_format_convertor::convert_array(
				pixel_format_color_rgba8, resolved_surface_->get_pixel_format(),
				dest_addr, src_addr,
				int(resolved_surface_->get_width())
				);
			src_addr += resolved_surface_->get_pitch();
		}

		resolved_surface_->unmap();

		glBindTexture(GL_TEXTURE_2D, tex_);
		if ((width_ < surface_width) || (height_ < surface_height))
		{
			width_	= static_cast<uint32_t>(surface_width);
			height_ = static_cast<uint32_t>(surface_height);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, &dest[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
				static_cast<GLsizei>(surface_width),
				static_cast<GLsizei>(surface_height),
				GL_RGBA, GL_UNSIGNED_BYTE, &dest[0]
			);
		}

		float fw = static_cast<float>(surface_width) / width_;
		float fh = static_cast<float>(surface_height) / height_;

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

		::SwapBuffers(dc_);
	}

private:
	HWND		window_;
	HDC			dc_;
	HGLRC		glrc_;
	GLuint		tex_;
	uint32_t	width_, height_;
};

swap_chain_ptr create_gl_swap_chain(
	renderer_ptr const& renderer,
	renderer_parameters const* params)
{
	if(!params)
	{
		return swap_chain_ptr();
	}

	return boost::make_shared<gl_swap_chain>(renderer, *params);
}

END_NS_SALVIAX();

#endif
