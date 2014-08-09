#include <salviau/include/win/win_application.h>

#include <salviau/src/win/resource.h>
#include <eflib/include/platform/constant.h>

BEGIN_NS_SALVIAU();

class win_window: public window
{
	signal< void() > on_idle;
	signal< void() > on_paint;
	signal< void() > on_create;
	
	void show()
	{
		ShowWindow( SW_SHOWDEFAULT );
	}

	void set_idle_handler( idle_handler_t const& handler )
	{
		on_idle.connect( handler );
	}

	void set_draw_handler( draw_handler_t const& handler )
	{
		m_view.on_paint.connect( handler );
	}

	void set_create_handler( create_handler_t const& handler )
	{
		m_view.on_create.connect( handler );
	}

	void set_title( string const& title )
	{
		SetWindowText( hwnd_, to_tstring(title).c_str() );
	}

	boost::any view_handle()
	{
		return boost::any(hwnd_);
	}

	void refresh()
	{
		InvalidateRect(hwnd_, nullptr);
	}
	
	bool create()
	{
		hwnd = CreateWindow("SalviaWinApp", "SalviaWinApp", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hinst, NULL);

		if (!wnd)
		{
			return false;
		}
		
		on_create();
		
		ShowWindow(wnd, SW_SHOW);
		UpdateWindow(wnd);

		return true;
	}
	
private:
	ATOM register_window_class(HINSTANCE hinst)
	{
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= &win_window::win_proc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hinst			= hinst;
		wcex.hIcon			= LoadIcon(hinst, MAKEINTRESOURCE(IDI_SALVIA_WIN_APP));
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SALVIA_WIN_APP);
		wcex.lpszClassName	= szWindowClass;
		wcex.hIconSm		= LoadIcon(wcex.hinst, MAKEINTRESOURCE(IDI_SMALL));

		return RegisterClassEx(&wcex);
	}
	
	static LRESULT CALLBACK win_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		for(window_map_)
		int msg_id, msg_event;
		PAINTSTRUCT ps;
		HDC hdc;

		switch (message)
		{
		case WM_COMMAND:
			msg_id    = LOWORD(wparam);
			msg_event = HIWORD(wparam);
			// Parse the menu selections:
			switch (msg_id)
			{
			case IDM_EXIT:
				DestroyWindow(wnd);
				break;
			default:
				return DefWindowProc(wnd, message, wparam, lparam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(wnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(wnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(wnd, message, wparam, lparam);
		}
		return 0;
	}
	
	ATOM	wnd_class_;
	HWND	hwnd_;
	
	static 	uint32_t const MAX_WINDOW_MAP_SIZE = 16;
	static  uint32_t current_wnd_;
	static	std::pair<HWND, win_window*> window_map_[MAX_WINDOW_MAP_SIZE];
};

class wtl_application: public application
{
public:
	wtl_application()
	{
		::DefWindowProc(NULL, 0, 0, 0L);
		
		HINSTANCE hinst = GetModuleHandle(nullptr);
		
		register_window_class(hinst);
		if ( !init_instance(hinst, nCmdShow) )
		{
			return FALSE;
		}
	}

	~wtl_application()
	{
		delete main_wnd;
		delete module;
	}

	window* main_window()
	{
		return main_wnd;
	}

	int run()
	{
		module->AddMessageLoop(&msg_loop);
		main_wnd->pmodule = module;

		if(main_wnd->CreateEx( NULL, NULL, WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU ) == NULL)
		{
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
};

application* create_win_application()
{
	return new win_application;
}

END_NS_SALVIAU();
