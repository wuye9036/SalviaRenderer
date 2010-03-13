// SRSampleWindow.cpp : main source file for SRSampleWindow.exe
//

#include "stdafx.h"

#include "eflib/include/slog.h"
#include <boost/smart_ptr.hpp>

using namespace efl;
using namespace std;
using namespace boost;

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>

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

	if(wndMain.CreateEx() == NULL)
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
	shared_ptr<_tfstream> hf(new _tfstream("log.txt", ios::out));
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
