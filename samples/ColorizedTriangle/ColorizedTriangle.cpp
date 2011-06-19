// ColorizedTriangle.cpp : main source file for ColorizedTriangle.exe
//

#include "stdafx.h"

#include <eflib/include/platform/config.h>
#include <eflib/include/platform/constant.h>

#include <eflib/include/platform/disable_warnings.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <eflib/include/platform/enable_warnings.h>

#include "resource.h"

#include "ColorizedTriangleView.h"
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
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, ::GetModuleHandle(NULL) );
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(NULL, SW_SHOWDEFAULT);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
