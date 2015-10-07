#include <iostream>
#include <process.h>
#include <string>
#include <memory>
#include <functional>
#include <fstream>

#include "lw_laywin.h"
#include <commctrl.h>

#include "stdio_redirector.h"

class X : public stdio_redirector {
public:
	X(laywin::edit* pe) {
		_pe = pe;
	}
private:
	laywin::edit* _pe;
protected:
	void write_stdout(LPCTSTR pszOutput) override {
		auto hwnd = _pe->hwnd();
		int len = ::GetWindowTextLength(hwnd);
		::SendMessage(hwnd, EM_SETSEL, len, len);
		::SendMessage(hwnd, EM_REPLACESEL, 0, LPARAM(pszOutput));
	}
};
class TW : public laywin::window_creator
{
public:
	TW()
	{}

protected:
	virtual LPCTSTR get_skin_json() const
	{
		LPCTSTR json =
#include "res/main.json"
			;
		return json;
	}

	virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled) override
	{
		static X* px;
		switch(umsg)
		{
		case WM_CREATE:
		{
			center();

			static X x(dynamic_cast<laywin::edit*>(_layout->find("output")));
			px = &x;
			x.open(".\\shadowsocks-local.exe -s=sss.twofei.com -p 5820 -k ssfwycfkdj -l=5820 -d");

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
