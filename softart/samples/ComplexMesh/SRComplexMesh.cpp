// SRComplexMesh.cpp : main source file for SRComplexMesh.exe
//

#include "stdafx.h"

#include "eflib/include/slog.h"
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif

using namespace eflib;
using namespace std;
using namespace boost;

#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 4996 6001 6011 6202 6225 6255 6309 6386 6387)
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

#include "resource.h"

#include "SRComplexMeshView.h"
#include "aboutdlg.h"
#include "MainFrm.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CGameLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	RECT rc = { 0, 0, 512, 512 };
	::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	rc.left = rc.top = 0;
	if(wndMain.CreateEx(NULL, rc) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.GameRun();

	_Module.RemoveMessageLoop();
	return nRet;
}

int main()
{
	boost::shared_ptr<_tfstream> hf(new _tfstream("log.txt", ios::out));
	typedef slog<text_log_serializer> slog_type;
	log_system<slog_type>::slog_type slog = log_system<slog_type>::instance(hf);
	slog.begin_log();
	slog.end_log();

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, ::GetModuleHandle(NULL));
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(NULL, SW_SHOWDEFAULT);

	_Module.Term();
	::CoUninitialize();
	
	hf->close();
	return nRet;
}
