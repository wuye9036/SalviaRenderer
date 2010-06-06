// stdafx.cpp : source file that includes just the standard includes
//	SRSampleWindow.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#include "eflib/include/eflib.h"

#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 6001 6011 6202 6225 6255 6309 6386 6387)
#endif
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif

#if (_ATL_VER < 0x0700)
#include <atlimpl.cpp>
#endif //(_ATL_VER < 0x0700)
