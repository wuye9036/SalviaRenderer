#pragma once

#include <salviax/include/presenter/presenter_forward.h>

#include <salviax/include/utility/inc_gdiplus.h>
#include <salviar/include/presenter_dev.h>
#include <salviar/include/framebuffer.h>

#include <eflib/include/utility/shared_declaration.h>

#include <algorithm>
#include <stdio.h>
#include <tchar.h>

BEGIN_NS_SALVIAX_PRESENTER();

EFLIB_DECLARE_CLASS_SHARED_PTR(dev_gdiplus);

class dev_gdiplus: public salviar::device
{
	dev_gdiplus(HWND hwnd);

	HWND hwnd_;
	boost::shared_ptr<Gdiplus::Bitmap> pbmp_;

public:
	~dev_gdiplus();
	static dev_gdiplus_ptr create_device(HWND hwnd);

	virtual void present(const salviar::surface& surf);
};

END_NS_SALVIAX_PRESENTER();

class gdiplus_initializer
{
	Gdiplus::GdiplusStartupInput gdiPlusInput_;
	ULONG_PTR gdiPlusToken_;
public:
	gdiplus_initializer()
	{
#ifdef _CONSOLE
		_tprintf(_T("%s"), _T("Gdiplus Initialized.\n"));
#else
		Gdiplus::GdiplusStartup(&gdiPlusToken_, &gdiPlusInput_, NULL);
#endif
	}
	~gdiplus_initializer()
	{
#ifdef _CONSOLE
		_tprintf(_T("%s"), _T("Gdiplus Shutdown.\n"));
#else
		Gdiplus::GdiplusShutdown(gdiPlusToken_);
#endif
	}
};

#ifdef salviax_gdiplus_presenter_EXPORTS
	#define SALVIAX_API __declspec(dllexport)
#else
	#define SALVIAX_API __declspec(dllimport)
#endif

extern "C"
{
	SALVIAX_API void salviax_create_presenter_device(salviar::device_ptr& dev, void* param);
}