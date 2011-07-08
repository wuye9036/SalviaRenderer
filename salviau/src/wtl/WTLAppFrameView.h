// WTLAppFrameView.h : interface of the CWTLAppFrameView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <eflib/include/platform/boost_begin.h>
#include <boost/signals2.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace boost::signals2;

class CWTLAppFrameView : public CWindowImpl<CWTLAppFrameView>
{
public:
	DECLARE_WND_CLASS(NULL)

	signal< void() > on_paint;
	signal< void() > on_create;

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP(CWTLAppFrameView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){
		on_create();
		return S_OK;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		on_paint();
		return 0;
	}
};
