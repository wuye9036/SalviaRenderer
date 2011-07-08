#include <eflib/include/platform/config.h>

#define NOMINMAX
#include <algorithm>
using std::min;
using std::max;

#include <salviau/src/wtl/stdafx.h>

#include <salviau/include/wtl/wtl_application.h>
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

#include <salviau/src/wtl/resource.h>

#include <salviau/src/wtl/WTLAppFrameView.h>
#include <salviau/src/wtl/aboutdlg.h>
#include <salviau/src/wtl/MainFrm.h>

CAppModule _Module;

BEGIN_NS_SALVIAU();

class CGameLoop : public CMessageLoop
{
public:
	int GameRun()
	{
		for(;;)
		{
			if (::PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))
			{
				if (WM_QUIT == m_msg.message)
				{
					ATLTRACE2(atlTraceUI, 0, _T("CGameLoop::Run - exiting\n"));
					break;        // WM_QUIT, exit message loop
				}

				if(!PreTranslateMessage(&m_msg))
				{
					::TranslateMessage(&m_msg);
					::DispatchMessage(&m_msg);
				}
			}
			else
			{
				OnIdle(0);
			}
		}

		return (int)m_msg.wParam;
	}
};

class wtl_application: public application{
public:
	wtl_application(){
		// Initalize module, window and message loop.
		HRESULT hRes = ::CoInitialize(NULL);
		ATLASSERT(SUCCEEDED(hRes));

		::DefWindowProc(NULL, 0, 0, 0L);

		module = &_Module;
		main_wnd = new CMainFrame();
	}

	~wtl_application(){
		delete main_wnd;
		::CoUninitialize();
	}

	window* main_window(){
		return main_wnd;
	}

	int run(){
		AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls
		HRESULT hRes = module->Init(NULL, ::GetModuleHandle(NULL) );
		ATLASSERT(SUCCEEDED(hRes));

		module->AddMessageLoop(&msg_loop);
		main_wnd->pmodule = module;

		if(main_wnd->CreateEx() == NULL)	{
			ATLTRACE(_T("Main window creation failed!\n"));
			return 0;
		}

		// Message loop.
		main_wnd->show();
		int nRet = msg_loop.GameRun();

		// Do clear while terminating.
		module->RemoveMessageLoop();
		module->Term();
		
		return nRet;
	}

private:
	CGameLoop	msg_loop;
	CMainFrame*	main_wnd;
	CAppModule*	module;
};

application* create_wtl_application(){
	return new wtl_application;
}

END_NS_SALVIAU();
