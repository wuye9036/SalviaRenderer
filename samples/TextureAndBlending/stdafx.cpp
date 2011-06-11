// stdafx.cpp : source file that includes just the standard includes
//	SRSampleWindow.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#include <eflib/include/platform/config.h>

#include <eflib/include/platform/disable_warnings.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <eflib/include/platform/enable_warnings.h>

#if (_ATL_VER < 0x0700)
#include <atlimpl.cpp>
#endif //(_ATL_VER < 0x0700)
