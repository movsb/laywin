#include <taowin/core/tw_syscontrols.h>
#include "button.h"

namespace taowin {

void button::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
{
    static style_map __known_styles[] =
    {
        {BS_MULTILINE, _T("multiline")},
        {BS_DEFPUSHBUTTON, _T("default")},
        {0, nullptr}
    };

    metas.known_styles = &__known_styles[0];
    metas.known_ex_styles = nullptr;
    metas.classname = WC_BUTTON;
}

bool button::filter_notify(int code, NMHDR* hdr, LRESULT* lr)
{
    if(code == BN_CLICKED) {
        if(_on_click) {
            _on_click();
            return true;
        }
    }

    return false;
}

option::option()
    : _b_has_group(false)
{
}

void option::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
    metas.classname = WC_BUTTON;
    metas.style |= BS_AUTORADIOBUTTON;
}

//////////////////////////////////////////////////////////////////////////
void check::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
    metas.classname = WC_BUTTON;
    metas.style |= BS_AUTOCHECKBOX;
}

bool check::get_chekc()
{
    return !!Button_GetCheck(_hwnd);
}

void check::set_attr(const TCHAR* name, const TCHAR* value)
{
    if(_tcscmp(name, _T("checked")) == 0) {
        bool checked = _tcscmp(value, _T("true")) == 0;
        ::SendMessage(_hwnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    else return __super::set_attr(name, value);
}

//////////////////////////////////////////////////////////////////////////
void label::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
    static style_map __known_styles[] =
    {
        {SS_CENTER, _T("center")},
        {SS_CENTERIMAGE, _T("centerimage")},
        {0, nullptr}
    };
    metas.classname = WC_STATIC;
    metas.known_styles = &__known_styles[0];
}

//////////////////////////////////////////////////////////////////////////
void group::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
    metas.style |= BS_GROUPBOX;
    metas.classname = WC_BUTTON;
}

}
