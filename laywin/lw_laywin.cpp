#include "lw_laywin.h"
#include "lw_parser.h"

namespace laywin{

	laywin::laywin()
		: _root(NULL)
	{

	}

	laywin::~laywin()
	{
		if(_root){
			delete _root;
			_root = NULL;
		}
	}

	void laywin::set_layout(LPCTSTR json, HWND hwnd)
	{
		parser par;
		_root = par.parse(json, &_mgr);

		_mgr.hwnd(hwnd);
		_root->manager_(&_mgr);

		_root->font(-2);
		_root->visible(_root->control::visible());

		_root->init();
	}

	void laywin::set_size(int cx, int cy)
	{
		if(_root && _root->size()){
			(*_root)[0]->pos(rect(0, 0, cx, cy));
		}
	}


	LPCTSTR window_creator::get_skin_json() const
	{
		return _T("");
	}

	LRESULT window_creator::handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled)
	{
		return __super::__handle_message(umsg, wparam, lparam, handled);
	}

	LRESULT window_creator::__handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled)
	{
		switch(umsg)
		{
		case WM_SIZE:
			_layout.set_size(LOWORD(lparam), HIWORD(lparam));
			break;
		case WM_CREATE:
			try{
				_layout.set_layout(get_skin_json(), _hwnd);
			}
			catch(LPCTSTR e){
				::MessageBox(_hwnd, e, NULL, MB_ICONERROR);
			}
			break;
		case WM_COMMAND:
		{
			HWND hwnd = HWND(lparam);
			int id = LOWORD(wparam);
			int code = HIWORD(wparam);

			if(hwnd == 0 && code == 0){
				if(id == 2){
					response_default_key_event(_hwnd, VK_ESCAPE);
					return 0;
				}
				else if(id == 1){
					response_default_key_event(_hwnd, VK_RETURN);
					return 0;
				}
			}

			if(hwnd){
				control* pc = _layout.root()->find(hwnd);
				//if(pc) return on_command_ctrl(hwnd, pc, code);
				return on_notify_ctrl(hwnd, pc, code, nullptr);
			}
			else{
				if(HIWORD(wparam) == 0){
					return on_menu(LOWORD(wparam));
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
			if(!lparam) break;
			NMHDR* hdr = reinterpret_cast<NMHDR*>(lparam);
			control* pc = _layout.root()->find(hdr->hwndFrom);
			if(!pc) break;
			return on_notify_ctrl(hdr->hwndFrom, pc, hdr->code, hdr);
		}
		}

		return handle_message(umsg, wparam, lparam, handled);
	}

	void window_creator::on_first_message(HWND hwnd)
	{
		__super::on_first_message(hwnd);
	}

	void window_creator::on_final_message(HWND hwnd)
	{
		return __super::on_final_message(hwnd);
	}

}
