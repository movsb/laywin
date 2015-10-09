#include "lw_window.h"

namespace laywin{

	array<window_manager*> window_manager::_window_managers;

	window_manager::window_manager()
		: _hwnd(NULL)
		, _message_filter(NULL)
		, _accelerator_translator(NULL)
	{

	}

	window_manager::~window_manager()
	{

	}

	void window_manager::init(HWND hwnd, i_message_filter* flt, i_accelerator_translator* trans)
	{
		_hwnd = hwnd;
		_message_filter = flt;
		_accelerator_translator = trans;
		add_window_manager(this);
	}

	void window_manager::deinit()
	{
		remove_window_manager(this);
	}

	bool window_manager::filter_message(MSG* pmsg)
	{
		return _message_filter && _message_filter->filter_message(
			pmsg->hwnd, pmsg->message, pmsg->wParam, pmsg->lParam);
	}

	bool window_manager::translate_accelerator(MSG* pmsg)
	{
		return _accelerator_translator 
			&& _accelerator_translator->translate_accelerator(pmsg);
	}

	void window_manager::message_loop()
	{
		MSG msg;
		while(::GetMessage(&msg, NULL, 0, 0)){
			if(!translate_meesage(&msg)){
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}

	bool window_manager::translate_meesage(MSG* pmsg)
	{
		bool bchild = !!(::GetWindowLongPtr(pmsg->hwnd, GWL_STYLE) & WS_CHILD);
		if(bchild){
			HWND hParent = pmsg->hwnd;
			while(hParent && ::GetWindowLongPtr(hParent, GWL_STYLE) & WS_CHILD){
				hParent = ::GetParent(hParent);
			}

			if(hParent != NULL){
				for(int i = 0; i < _window_managers.size(); i++){
					window_manager* mgr = _window_managers[i];
					if(mgr->hwnd() == hParent){
						return mgr->translate_accelerator(pmsg)
							|| mgr->filter_message(pmsg);
					}
				}
			}
		}
		else{
			for(int i = 0; i < _window_managers.size(); i++){
				window_manager* mgr = _window_managers[i];
				if(mgr->hwnd() == pmsg->hwnd){
					return mgr->translate_accelerator(pmsg)
						|| mgr->filter_message(pmsg);
				}
			}
		}

		return false;
	}


	window::window()
		: _hwnd(NULL)
		, _old_wnd_proc(NULL)
		, _b_subclassed(false)
		, _b_has_mgr(false)
	{

	}

	window::~window()
	{

	}

	HWND window::create(HWND hParent, DWORD dwStyle, DWORD dwExStyle, HMENU hMenu /*= NULL*/)
	{
		if(get_super_class_name() && !register_super_class()) return NULL;
		if(!get_super_class_name() && !register_window_class()) return NULL;

		_hwnd = ::CreateWindowEx(dwExStyle, get_window_class_name(), get_window_name(), get_window_style(),
			0, 0, 300, 300, hParent, hMenu, GetModuleHandle(0), this);
		assert(_hwnd);
		return _hwnd;
	}

	bool window::do_modal(HWND owner)
	{
		return false;
	}

	bool window::do_modeless(HWND owner)
	{
		return false;
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

	bool window::register_window_class()
	{
		WNDCLASS wc = {0};
		wc.style = get_class_style();
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hIcon = NULL;
		wc.lpfnWndProc = __window_procedure;
		wc.hInstance = GetModuleHandle(0);
		wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = get_class_brush();
		wc.lpszMenuName = NULL;
		wc.lpszClassName = get_window_class_name();
		ATOM ret = ::RegisterClass(&wc);
		assert(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
		return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
	}

	bool window::register_super_class()
	{
		WNDCLASSEX wc = {0};
		wc.cbSize = sizeof(WNDCLASSEX);
		if(!::GetClassInfoEx(NULL, get_super_class_name(), &wc)) {
			if(!::GetClassInfoEx(GetModuleHandle(0), get_super_class_name(), &wc)) {
				assert(!"Unable to locate window class");
				return NULL;
			}
		}
		_old_wnd_proc = wc.lpfnWndProc;
		wc.lpfnWndProc = __window_procedure;
		wc.hInstance = GetModuleHandle(0);
		wc.lpszClassName = get_window_class_name();
		ATOM ret = ::RegisterClassEx(&wc);
		assert(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
		return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
	}

	HWND window::subclass(HWND hwnd)
	{
		assert(::IsWindow(_hwnd));
		assert(_hwnd == NULL);
		_old_wnd_proc = (WNDPROC)SetWindowLongPtr(_hwnd, GWL_WNDPROC, (LONG)__control_procedure);
		if(_old_wnd_proc == NULL) return NULL;
		_b_subclassed = true;
		_hwnd = hwnd;
		::SetProp(_hwnd, _T("WndX"), this);
		return _hwnd;
	}

	void window::unsubclass()
	{
		assert(::IsWindow(_hwnd));
		if(!::IsWindow(_hwnd)) return;
		if(!_b_subclassed) return;
		SetWindowLongPtr(_hwnd, GWL_WNDPROC, (LONG)_old_wnd_proc);
		_old_wnd_proc = ::DefWindowProc;
		_b_subclassed = false;
	}

	LRESULT window::__handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled)
	{
		if(_type == window_type::modal_dialog
			|| _type == window_type::modeless_dialog)
		{
			if(umsg == WM_CLOSE){
				if(_type == window_type::modal_dialog){
					::EndDialog(_hwnd, 0);
				}
				else{
					::DestroyWindow(_hwnd);
				}
				return 0;
			}
			else{
				handled = false;
				return 0;
			}
		}
		return ::CallWindowProc(_old_wnd_proc, _hwnd, umsg, wparam, lparam);
	}

	void window::on_first_message(HWND hwnd)
	{
		DWORD dwStyle = ::GetWindowLongPtr(hwnd, GWL_STYLE);
		if(dwStyle & WS_CHILD && !(dwStyle & WS_POPUP)){
			_b_has_mgr = false;
		}
		else{
			_b_has_mgr = true;
			_wndmgr.init(hwnd, this, this);
		}
	}

	void window::on_final_message(HWND hwnd)
	{
		if(_b_has_mgr){
			_wndmgr.deinit();
			_b_has_mgr = false;
		}
	}

	bool window::response_default_key_event(HWND hChild, WPARAM wParam)
	{
		if(wParam == VK_ESCAPE){
			close();
			return true;
		}
		return false;
	}

	LRESULT __stdcall window::__window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		window* pThis = NULL;
		if(umsg == WM_NCCREATE) {
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
			pThis = static_cast<window*>(lpcs->lpCreateParams);
			pThis->_hwnd = hwnd;
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
			pThis->on_first_message(hwnd);
		}
		else {
			pThis = reinterpret_cast<window*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if(umsg == WM_NCDESTROY && pThis != NULL) {
				LRESULT lRes = ::CallWindowProc(pThis->_old_wnd_proc, hwnd, umsg, wparam, lparam);
				::SetWindowLongPtr(pThis->_hwnd, GWLP_USERDATA, 0L);
				if(pThis->_b_subclassed) pThis->unsubclass();
				pThis->on_final_message(hwnd);
				return lRes;
			}
		}

		if(pThis != NULL) {
			bool bHandled = false;
			LRESULT r = pThis->__handle_message(umsg, wparam, lparam, bHandled);
			if(bHandled) return r;
		}
		return ::DefWindowProc(hwnd, umsg, wparam, lparam);
	}

	LRESULT __stdcall window::__control_procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		window* pThis = NULL;
		if(uMsg == WM_NCCREATE) {
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			pThis = static_cast<window*>(lpcs->lpCreateParams);
			::SetProp(hWnd, _T("WndX"), (HANDLE)pThis);
			pThis->_hwnd = hWnd;
			pThis->on_first_message(hWnd);
		}
		else {
			pThis = reinterpret_cast<window*>(::GetProp(hWnd, _T("WndX")));
			if(uMsg == WM_NCDESTROY && pThis != NULL) {
				LRESULT lRes = ::CallWindowProc(pThis->_old_wnd_proc, hWnd, uMsg, wParam, lParam);
				if(pThis->_b_subclassed) pThis->unsubclass();
				::SetProp(hWnd, _T("WndX"), NULL);
				pThis->_hwnd = NULL;
				pThis->on_final_message(hWnd);
				return lRes;
			}
		}
		if(pThis != NULL) {
			bool bHandled = false;
			LRESULT r = pThis->__handle_message(uMsg, wParam, lParam, bHandled);
			if(bHandled) return r;
		}
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	INT_PTR __stdcall window::__dialog_procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		window* pThis = NULL;
		if(uMsg == WM_INITDIALOG){
			pThis = (window*)(lParam);
			pThis->_hwnd = hWnd;
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
			pThis->on_first_message(hWnd);
		}
		else{
			pThis = reinterpret_cast<window*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
			if(uMsg == WM_NCDESTROY && pThis != NULL) {
				LRESULT lRes = ::CallWindowProc(pThis->_old_wnd_proc, hWnd, uMsg, wParam, lParam);
				::SetWindowLongPtr(pThis->_hwnd, GWLP_USERDATA, 0L);
				pThis->_hwnd = NULL;
				pThis->on_final_message(hWnd);
				return lRes;
			}
		}

		if(pThis != NULL){
			bool bHandled = false;
			LRESULT r = pThis->__handle_message(uMsg, wParam, lParam, bHandled);
			if(bHandled){
				return SetWindowLongPtr(hWnd, DWL_MSGRESULT, LONG(r));
			}
		}
		return FALSE;
	}

	void window::close()
	{
		send_message(WM_CLOSE);
	}

}
