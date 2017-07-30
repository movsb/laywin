#include "tw_window.h"
#include "tw_syscontrols.h"

namespace taowin{

    WindowManager __window_manager;

	Window::Window()
		: _hwnd(NULL)
        , _return_code(0)
	{

	}

	Window::~Window()
	{

	}

	HWND Window::create(HWND owner)
	{
        WindowMeta metas;
        get_metas(&metas);
		_hwnd = ::CreateWindowEx(metas.exstyle, metas.classname, metas.caption, metas.style,
            100, 100, 300, 250, owner, nullptr, nullptr, this);
		assert(_hwnd);
		return _hwnd;
	}

	int Window::domodal(HWND owner)
	{
        assert(owner != nullptr);
        WindowMeta metas;
        get_metas(&metas);
        metas.style |= WS_VISIBLE;
		_hwnd = ::CreateWindowEx(metas.exstyle, metas.classname, metas.caption, metas.style,
            100, 100, 300, 250, owner, nullptr, nullptr, this);
		assert(_hwnd);
        ::EnableWindow(owner, FALSE);
        return __window_manager.loop_message();
	}

	void Window::show(bool show /*= true*/, bool focus /*= true*/)
	{
		assert(::IsWindow(_hwnd));
		::ShowWindow(_hwnd, show ? (focus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
	}

	void Window::center()
	{
		assert(::IsWindow(_hwnd));
		assert((GetWindowLongPtr(_hwnd, GWL_STYLE)&WS_CHILD) == 0);
		RECT rcDlg = {0};
		::GetWindowRect(_hwnd, &rcDlg);
		RECT rcArea = {0};
		RECT rcCenter = {0};
		HWND hWnd = *this;
		HWND hWndCenter = ::GetWindow(_hwnd, GW_OWNER);
		if(hWndCenter != NULL)
			hWnd = hWndCenter;

		// 处理多显示器模式下屏幕居中
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
		rcArea = oMonitor.rcWork;

		if(hWndCenter == NULL)
			rcCenter = rcArea;
		else
			::GetWindowRect(hWndCenter, &rcCenter);

		int DlgWidth = rcDlg.right - rcDlg.left;
		int DlgHeight = rcDlg.bottom - rcDlg.top;

		// Find dialog's upper left based on rcCenter
		int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
		int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

		// The dialog is outside the screen, move it inside
		if(xLeft < rcArea.left) xLeft = rcArea.left;
		else if(xLeft + DlgWidth > rcArea.right) xLeft = rcArea.right - DlgWidth;
		if(yTop < rcArea.top) yTop = rcArea.top;
		else if(yTop + DlgHeight > rcArea.bottom) yTop = rcArea.bottom - DlgHeight;
		::SetWindowPos(_hwnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	LRESULT Window::__handle_message(UINT umsg, WPARAM wparam, LPARAM lparam)
	{
        if(umsg == WM_CLOSE) {
            HWND owner = ::GetWindow(_hwnd, GW_OWNER);
            if(owner) {
                if (!::IsWindowEnabled(owner)) {
                    ::PostQuitMessage(_return_code);
                    ::EnableWindow(owner, TRUE);
                }
                ::SetActiveWindow(owner);
            }
        }
        return ::DefWindowProc(_hwnd, umsg, wparam, lparam);
	}

	void Window::on_first_message()
	{

	}

	void Window::on_final_message()
	{

	}

	LRESULT __stdcall Window::__window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		Window* pThis = reinterpret_cast<Window*>(::GetWindowLongPtr(hwnd, 4));

        // 窗口创建的第一条消息
		if(umsg == WM_NCCREATE) {
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
			pThis = static_cast<Window*>(lpcs->lpCreateParams);
			pThis->_hwnd = hwnd;
			::SetWindowLongPtr(hwnd, 4, reinterpret_cast<LPARAM>(pThis));
            __window_manager.add_message_filter(pThis);
			pThis->on_first_message();
            return TRUE; // must
		}
        // 窗口被销毁的最后一条消息（不包括投递的消息）
        else if(umsg == WM_NCDESTROY) {
            ::SetWindowLongPtr(pThis->_hwnd, 4, 0);
            ::DefWindowProc(hwnd, umsg, wparam, lparam);
            if(pThis) {
                __window_manager.remove_message_filter(pThis);
                pThis->on_final_message();
            }
            return 0;
		}

        return pThis
            ? pThis->__handle_message(umsg, wparam, lparam)
            : ::DefWindowProc(hwnd, umsg, wparam, lparam);
	}

	void Window::close(int code)
	{
        _return_code = code;
		send_message(WM_CLOSE);
	}

    void Window::get_metas(WindowMeta* metas) {
        // 默认的窗口标题与窗口类名
        metas->caption = _T("taowin");
        metas->classname = _T("taowin::Window");

        // 默认的窗口风格
        metas->style = WS_OVERLAPPEDWINDOW;
        metas->exstyle = WS_EX_APPWINDOW;

        // 默认的窗口标志
        metas->flags = WindowFlag::center;
    }

    //////////////////////////////////////////////////////////////////////////
    void register_window_classes() {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(wc);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
        wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);

        // TODO 这个ICON是乱写的
        wc.hIcon = wc.hIconSm = (HICON)::LoadImage(::GetModuleHandle(nullptr), (LPCTSTR)101, IMAGE_ICON, 32, 32, 0);
        wc.hInstance = ::GetModuleHandle(nullptr);
        wc.lpfnWndProc = &Window::__window_procedure;
        wc.lpszClassName = _T("taowin::Window");
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.cbWndExtra = sizeof(void*)* 2; // [[extra_ptr][this]]
        ::RegisterClassEx(&wc);
    }
}
