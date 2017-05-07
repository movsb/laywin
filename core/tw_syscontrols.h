#ifndef __taowin_syscontrols_h__
#define __taowin_syscontrols_h__

#include <map>
#include <vector>
#include <limits.h>
#include <functional>
#include <memory>

#include <windows.h>
#include <CommCtrl.h>
#include <WindowsX.h>
#include <MsHTML.h>

#include "../taolog.h"

#include "tw_control.h"
#include "tw_util.h"

namespace taowin{

    #define is_attr(a) (_tcscmp(name, a) == 0)

	struct style_map
	{
		DWORD dwStyle;
		LPCTSTR strStyle;
	};

    struct syscontrol_metas {
        DWORD           style;
        DWORD           exstyle;
        const TCHAR*    caption;
        const TCHAR*     classname;
        style_map*      known_styles;
        style_map*      known_ex_styles;
        std::function<void()> after_created;
        std::function<void()> before_creation;

        syscontrol_metas() {
            style = WS_CHILD | WS_VISIBLE;
            exstyle = 0;
            caption = _T("");
            classname = _T("");
            known_styles = nullptr;
            known_ex_styles = nullptr;
        }
    };

    typedef std::function<LRESULT()> OnNotify;
    typedef std::function<LRESULT(NMHDR*)> OnHdrNotify;

    class window_creator;

	class syscontrol : public control
	{
        friend class window_creator;

	public:
		syscontrol();

	public:
        void create(HWND parent, std::map<string, string>& attrs, resmgr& mgr) override;
        void attach(HWND hwnd) { _hwnd = hwnd; }
        unsigned int get_ctrl_id() const;

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) = 0;
        virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) { return false; }
        virtual bool filter_child(HWND child, int code, NMHDR* hdr, LRESULT* lr) { return false; }

    private:
        void create_metas(syscontrol_metas& metas, std::map<string, string>& attrs);

    private:
        window_creator* _owner;
        WNDPROC _old_wnd_proc;
	};

    class custom_control : public syscontrol
    {
    protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
        {
            static bool registered = false;
            if(!registered) {
                register_window_classes();
                registered = true;
            }
            metas.classname = _T("taowin::control");
        }

        static LRESULT __stdcall __control_procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual bool control_procedure(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lr) = 0;
        void register_window_classes();
    };

}

#endif//__taowin_syscontrols_h__
