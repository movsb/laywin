#include <taowin/core/tw_syscontrols.h>
#include "Progress.h"

namespace taowin {
void Progress::set_range(int min, int max)
{
    ::SendMessage(_hwnd, PBM_SETRANGE32, min, max);
}

void Progress::set_pos(int pos)
{
    ::SendMessage(_hwnd, PBM_SETPOS, pos, 0);
}

void Progress::set_bkcolor(COLORREF color)
{
    ::SendMessage(_hwnd, PBM_SETBKCOLOR, 0, color);
}

void Progress::set_color(COLORREF color)
{
    ::SendMessage(_hwnd, PBM_SETBARCOLOR, 0, color);
}

void Progress::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs)
{
    metas.classname = PROGRESS_CLASS;
    metas.after_created = [this] {
        ::SendMessage(_hwnd, PBM_SETSTEP, 1, 0);
    };
}

void Progress::set_attr(const TCHAR* name, const TCHAR* value)
{
    if(is_attr(_T("color"))) {
        int r, g, b;
        if(_stscanf(value, _T("%d,%d,%d"), &r, &g, &b) == 3) {
            set_color(RGB(r, g, b));
        }
    }
    else if(is_attr(_T("bkcolor"))) {
        int r, g, b;
        if(_stscanf(value, _T("%d,%d,%d"), &r, &g, &b) == 3) {
            set_bkcolor(RGB(r, g, b));
        }
    }
    else if(is_attr(_T("range"))) {
        int min, max;
        if(_stscanf(value, _T("%d,%d"), &min, &max) == 2) {
            set_range(min, max);
        }
    }
    else {
        return __super::set_attr(name, value);
    }
}

}