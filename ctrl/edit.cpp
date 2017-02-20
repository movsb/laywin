#include <core/tw_syscontrols.h>
#include "edit.h"

namespace taowin {

void edit::set_sel(int start, int end)
{
    Edit_SetSel(_hwnd, start, end);
}

void edit::replace_sel(const TCHAR* s)
{
    Edit_ReplaceSel(_hwnd, s);
}

void edit::append(const TCHAR* s)
{
    set_sel(size(), -1);
    replace_sel(s);
}

int edit::size() const
{
    return ::GetWindowTextLength(_hwnd);
}

void edit::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
{
    static style_map __known_styles[] =
    {
        {ES_CENTER, _T("center")},
        {ES_MULTILINE, _T("multiline")},
        {ES_NOHIDESEL, _T("nohidesel")},
        {ES_NUMBER, _T("number")},
        {ES_READONLY, _T("readonly")},
        {ES_WANTRETURN, _T("wantreturn")},
        {0, nullptr}
    };

    metas.classname = WC_EDIT;
    metas.style |= ES_AUTOHSCROLL | ES_AUTOVSCROLL;
    metas.known_styles = &__known_styles[0];
}

bool edit::filter_notify(int code, NMHDR* hdr, LRESULT* lr)
{
    if(code == EN_CHANGE) {
        if(_on_change) {
            *lr = _on_change();
            return true;
        }
    }

    return false;
}


}