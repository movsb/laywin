#ifndef __taowin_h__
#define __taowin_h__

#include <functional>

#include <windows.h>
#include <CommCtrl.h>
#include <windowsx.h>

#include <taowin/core/tw_util.h>
#include <taowin/core/tw_control.h>
#include <taowin/core/tw_syscontrols.h>
#include <taowin/core/tw_window.h>
#include <taowin/core/tw_resmgr.h>
#include <taowin/core/tw_menu.h>

#include <taowin/ctrl/button.h>
#include <taowin/ctrl/combobox.h>
#include <taowin/ctrl/TextBox.h>
#include <taowin/ctrl/header.h>
#include <taowin/ctrl/listview.h>
#include <taowin/ctrl/Progress.h>
#include <taowin/ctrl/TabCtrl.h>
#include <taowin/ctrl/webview.h>

#include "../taolog.h"

namespace taowin{

    class WindowCreator : public Window
	{
    public:
        WindowCreator();
        virtual ~WindowCreator();

        void subclass_control(SystemControl* ctl);
        void add_menu(const MenuManager* menu) { _menus.add(menu); }

	protected:
		virtual LPCTSTR get_skin_xml() const;
		virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);
		virtual LRESULT control_message(SystemControl* ctl, UINT umsg, WPARAM wparam, LPARAM lparam);
        virtual SystemControl* filter_control(HWND hwnd) { return nullptr; }
		virtual LRESULT on_menu(const MenuIds& ids) { return 0; }
        virtual LRESULT on_accel(int id) { return 0; }
		virtual LRESULT on_notify(HWND hwnd, Control* pc, int code, NMHDR* hdr) { return 0; }

        void async_call(std::function<void()> fn) {
            __window_manager.add_async_call(fn);
        }

	private:
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
        static LRESULT __stdcall __control_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	protected:
        bool            _window_created;
        WindowContainer* _window;
        RootControl*   _root;
        ResourceManager          _mgr;
        Array<const MenuManager*> _menus;
	};

    void init();
    int loop_message();
}

#endif//__taowin_h__
