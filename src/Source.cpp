#include <iostream>
#include <process.h>
#include <string>
#include <memory>
#include <functional>
#include <fstream>

#include "lw_laywin.h"
#include <commctrl.h>

class TW : public laywin::window_creator
{
public:
	TW()
	{}

protected:
	virtual LPCTSTR get_skin_json() const
	{
		LPCTSTR json =
#include "../res/main.json"
			;
		return json;
	}

	virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled) override
	{
		switch(umsg)
		{
		case WM_CREATE:
		{
			center();

			handled = true;
			return 0;
		}
		default:
			break;
		}
		return 0;
	}

	virtual LRESULT on_notify_ctrl(HWND hwnd, laywin::control* pc, int code, NMHDR* hdr) override
	{
		return __super::on_notify_ctrl(hwnd, pc, code, hdr);
	}
};

#ifdef _DEBUG
int main()
#else
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#endif
{
	try{
		TW tw1;
		tw1.create();
		tw1.show();

		laywin::window_manager::message_loop();
	}
	catch(LPCTSTR e){
		std::wcout << e << std::endl;
	}
	
	return 0;
}
