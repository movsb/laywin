#include "lw_window.h"

namespace laywin{

	array<i_message_filter*> window_manager::_message_filters;

	window::window()
		: _hwnd(NULL)
        , _is_dialog(false)
	{

	}

	window::~window()
	{

	}

	HWND window::create(const window_meta_t& meta)
	{
		_hwnd = ::CreateWindowEx(0, meta.classname, meta.caption, WS_OVERLAPPEDWINDOW,
            0, 0, 100, 100, nullptr, nullptr, nullptr, this);
		assert(_hwnd);
		return _hwnd;
	}

	int window::domodal(const window_meta_t& meta, HWND owner)
	{
        _is_dialog = true;
		_hwnd = ::CreateWindowEx(meta.exstyle, meta.classname, meta.caption, meta.style,
            0, 0, 100, 100, owner, nullptr, nullptr, this);
		assert(_hwnd);
        show();
        ::EnableWindow(owner, FALSE);

        return dialog_manager(this).run();
	}

	void window::show(bool show /*= true*/, bool focus /*= true*/)
	{
		assert(::IsWindow(_hwnd));
		::ShowWindow(_hwnd, show ? (focus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
	}

	void window::center()
	{
		assert(::IsWindow(_hwnd));
		assert((GetWindowLongPtr(_hwnd, GWL_STYLE)&WS_CHILD) == 0);
		RECT rcDlg = {0};
		::GetWindowRect(_hwnd, &rcDlg);
		RECT rcArea = {0};
		RECT rcCenter = {0};
		HWND hWnd = *this;
		HWND hWndParent = ::GetParent(_hwnd);
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

	LRESULT window::__handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled)
	{
        if(umsg == WM_CLOSE) {
            HWND owner = ::GetWindow(_hwnd, GW_OWNER);
            if(owner && _is_dialog) {
                ::PostQuitMessage(0);
                ::EnableWindow(owner, TRUE);
                ::SetActiveWindow(owner);
            }
        }
        return 0;
	}

	void window::on_first_message()
	{

	}

	void window::on_final_message()
	{

	}

	LRESULT __stdcall window::__window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		window* pThis = nullptr;
		if(umsg == WM_NCCREATE) {
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
			pThis = static_cast<window*>(lpcs->lpCreateParams);
			pThis->_hwnd = hwnd;
			::SetWindowLongPtr(hwnd, 4, reinterpret_cast<LPARAM>(pThis));
            if(!pThis->_is_dialog)
                window_manager::_message_filters.add(pThis);
			pThis->on_first_message();
		}
		else {
			pThis = reinterpret_cast<window*>(::GetWindowLongPtr(hwnd, 4));
			if(umsg == WM_NCDESTROY && pThis != nullptr) {
				::SetWindowLongPtr(pThis->_hwnd, 4, 0);
				LRESULT lRes = ::DefWindowProc(hwnd, umsg, wparam, lparam);
                if(!pThis->_is_dialog)
                    window_manager::_message_filters.remove(pThis);
				pThis->on_final_message();
				return lRes;
			}
		}

		if(pThis != nullptr) {
			bool bHandled = false;
			LRESULT r = pThis->__handle_message(umsg, wparam, lparam, bHandled);
			if(bHandled) return r;
		}

		return ::DefWindowProc(hwnd, umsg, wparam, lparam);
	}

	void window::close()
	{
		send_message(WM_CLOSE);
	}

    //////////////////////////////////////////////////////////////////////////
    void register_window_classes() {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(wc);
        wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
        wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = wc.hIconSm = ::LoadIcon(nullptr, IDI_APPLICATION);
        wc.hInstance = ::GetModuleHandle(nullptr);
        wc.lpfnWndProc = &window::__window_procedure;
        wc.lpszClassName = "taowin";
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.cbWndExtra = sizeof(void*)* 2; // [[extra_ptr][this]]
        ::RegisterClassEx(&wc);
    }
}
