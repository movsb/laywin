#include <assert.h>
#include <windows.h>
#include <CommCtrl.h>
#include <WindowsX.h>
#include <vector>

#include "tw_control.h"
#include "tw_resmgr.h"
#include "tw_syscontrols.h"

namespace taowin{

    int __map_style(DWORD* dwStyle, style_map* known_styles, std::vector<string>& styles)
    {
        int n = 0;
        for(auto& style : styles){
            if(style.size()){
                for(int i = 0; known_styles[i].strStyle != nullptr; i++){
                    if(_tcscmp(known_styles[i].strStyle, style.c_str()) == 0){
                        *dwStyle |= known_styles[i].dwStyle;
                        style = _T("");
                        n++;
                        break;
                    }
                }
            }
        }
        return n;
    }

	SystemControl::SystemControl()
        : _old_wnd_proc(0)
        , _owner(0)
	{

	}

    void SystemControl::create(HWND parent, std::map<string, string>& attrs, ResourceManager& mgr) {
        SystemControlMetas metas;
        create_metas(metas, attrs);

        if(metas.before_creation)
            metas.before_creation();

        _hwnd = ::CreateWindowEx(metas.exstyle, metas.classname, metas.caption, metas.style,
            0, 0, 0, 0, parent, HMENU(mgr.next_ctrl_id()), nullptr, this);
        assert(_hwnd);
        if(!_hwnd) return;

        _owner = mgr._owner;

        if(metas.after_created)
            metas.after_created();

        decltype(attrs.begin()) it;

        it = attrs.find(_T("font"));
        HFONT font = it == attrs.end()
            ? mgr.get_font(_T("default"))
            : mgr.get_font(it->second.c_str());
        ::SendMessage(_hwnd, WM_SETFONT, WPARAM(font), TRUE);
        if(it != attrs.end())
            attrs.erase(it);

        it = attrs.find(_T("text"));
        if(it != attrs.end()) {
            attrs.erase(it);
        }

        for(auto it = attrs.cbegin(); it != attrs.cend(); it++)
            set_attr(it->first.c_str(), it->second.c_str());
    }

    unsigned int SystemControl::get_ctrl_id() const
    {
        return ::GetWindowLongPtr(_hwnd, GWL_ID);
    }

    void SystemControl::create_metas(SystemControlMetas& metas, std::map<string, string>& attrs)
    {
        static style_map __known_styles[] =
        {
            {WS_BORDER, _T("border")},
            {WS_CAPTION, _T("caption")},
            {WS_CHILD, _T("child")},
            {WS_CLIPSIBLINGS, _T("clipsiblings")},
            {WS_CLIPCHILDREN, _T("clipchildren")},
            {WS_DISABLED, _T("disabled")},
            {WS_GROUP, _T("group")},
            {WS_HSCROLL, _T("hscroll")},
            {WS_TABSTOP, _T("tabstop")},
            {WS_VSCROLL, _T("vscroll")},
            {0, nullptr}
        };

        static style_map __known_ex_styles[] =
        {
            {WS_EX_ACCEPTFILES, _T("acceptfiles")},
            {WS_EX_CLIENTEDGE, _T("clientedge")},
            {WS_EX_STATICEDGE, _T("staticedge")},
            {WS_EX_TOOLWINDOW, _T("toolwindow")},
            {WS_EX_TOPMOST, _T("topmost")},
            {WS_EX_TRANSPARENT, _T("transparent")},
            {0, nullptr}
        };
        
        get_metas(metas, attrs);        
        
        decltype(attrs.begin()) it;

        if((it = attrs.find(_T("style"))) != attrs.end()) {
            std::vector<string> styles;
            split_string(&styles, it->second.c_str());

            int mapped = 0;
            if(metas.known_styles)
                mapped += __map_style(&metas.style, metas.known_styles, styles);
            mapped += __map_style(&metas.style, &__known_styles[0], styles);

            assert(mapped == (int)styles.size());
            attrs.erase(it);
        }

        if((it = attrs.find(_T("exstyle"))) != attrs.end()) {
            std::vector<string> exstyles;
            split_string(&exstyles, it->second.c_str());

            int mapped = 0;
            if(metas.known_ex_styles)
                mapped += __map_style(&metas.exstyle, metas.known_ex_styles, exstyles);
            mapped += __map_style(&metas.exstyle, &__known_ex_styles[0], exstyles);

            assert(mapped == (int)exstyles.size());
            attrs.erase(it);
        }

        if((it = attrs.find(_T("text"))) != attrs.end()) {
            auto& text = it->second;
            metas.caption = text.c_str();   // TODO ALERT text is not local!
            // DO not erase it.
        }
    }

    LRESULT __stdcall CustomControl::__control_procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CustomControl* pThis = static_cast<CustomControl*>(reinterpret_cast<SystemControl*>(::GetWindowLongPtr(hWnd, 4)));

        if(uMsg == WM_NCCREATE) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pThis = static_cast<CustomControl*>(static_cast<SystemControl*>(lpcs->lpCreateParams));
            pThis->hwnd(hWnd);
            ::SetWindowLongPtr(hWnd, 4, reinterpret_cast<LPARAM>(pThis));
            return TRUE; // must
        }
        else if(uMsg == WM_NCDESTROY) {
            ::SetWindowLongPtr(pThis->hwnd(), 4, 0);
            ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            return 0;
        }

        LRESULT lr;
        if(pThis && pThis->control_procedure(uMsg, wParam, lParam, lr))
            return lr;
        else
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    void CustomControl::register_window_classes()
    {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(wc);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = wc.hIconSm = (HICON)::LoadImage(::GetModuleHandle(nullptr), (LPCTSTR)101, IMAGE_ICON, 32, 32, 0);
        wc.hInstance = ::GetModuleHandle(nullptr);
        wc.lpfnWndProc = &__control_procedure;
        wc.lpszClassName = _T("taowin::control");
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.cbWndExtra = sizeof(void*) * 2; // [[extra_ptr][this]]
        ::RegisterClassEx(&wc);
    }

}
