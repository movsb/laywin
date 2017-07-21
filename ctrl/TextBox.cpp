#include <taowin/core/tw_syscontrols.h>
#include "TextBox.h"

namespace taowin {

void TextBox::set_sel(int start, int end)
{
    Edit_SetSel(_hwnd, start, end);
}

void TextBox::replace_sel(const TCHAR* s)
{
    Edit_ReplaceSel(_hwnd, s);
}

void TextBox::append(const TCHAR* s)
{
    set_sel(size(), -1);
    replace_sel(s);
}

int TextBox::size() const
{
    return ::GetWindowTextLength(_hwnd);
}

void TextBox::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs)
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

bool TextBox::filter_notify(int code, NMHDR* hdr, LRESULT* lr)
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