// SRSampleWindow.cpp : main source file for SRSampleWindow.exe
//

#include "stdafx.h"

#include <eflib/include/diagnostics/log.h>

#include <eflib/include/platform/disable_warnings.h>

#include <boost/smart_ptr.hpp>

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>

#include <eflib/include/platform/enable_warnings.h>

using namespace eflib;
using namespace std;
using namespace boost;

#include "resource.h"

#include "SRSampleWindowView.h"
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
	typedef eflib::log<text_log_serializer> log_t;
	log_system<log_t>::log_t logsys = log_system<log_t>::instance(hf);
	logsys.begin_log();
	logsys.end_log();

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
