#ifndef __taowin_h__
#define __taowin_h__

#include <functional>

#include <windows.h>
#include <CommCtrl.h>
#include <windowsx.h>

#include "tw_util.h"
#include "tw_control.h"
#include "tw_syscontrols.h"
#include "tw_window.h"
#include "tw_resmgr.h"

namespace taowin{

    class window_creator : public window
	{
    public:
        window_creator();
        virtual ~window_creator();

        void subclass_control(syscontrol* ctl);

	protected:
		virtual LPCTSTR get_skin_xml() const;
		virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);
		virtual LRESULT control_message(syscontrol* ctl, UINT umsg, WPARAM wparam, LPARAM lparam);
		virtual LRESULT on_menu(int id, bool is_accel = false) { return 0; }
		virtual LRESULT on_notify(HWND hwnd, control* pc, int code, NMHDR* hdr) { return 0; }

        void async_call(std::function<void()> fn) {
            __window_manager.add_async_call(fn);
        }

	private:
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
        static LRESULT __stdcall __control_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	protected:
        bool            _window_created;
        window_container* _window;
        root_control*   _root;
        resmgr          _mgr;
	};

    void init();
    int loop_message();
}

#endif//__taowin_h__
