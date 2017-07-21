#include <taowin/core/tw_syscontrols.h>
#include "button.h"

namespace taowin {

void Button::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs)
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

bool Button::filter_notify(int code, NMHDR* hdr, LRESULT* lr)
{
    if(code == BN_CLICKED) {
        if(_on_click) {
            _on_click();
            return true;
        }
    }

    return false;
}

RadioButton::RadioButton()
    : _b_has_group(false)
{
}

void RadioButton::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) {
    metas.classname = WC_BUTTON;
    metas.style |= BS_AUTORADIOBUTTON;
}

//////////////////////////////////////////////////////////////////////////
void CheckBox::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) {
    metas.classname = WC_BUTTON;
    metas.style |= BS_AUTOCHECKBOX;
}

bool CheckBox::get_chekc()
{
    return !!Button_GetCheck(_hwnd);
}

void CheckBox::set_attr(const TCHAR* name, const TCHAR* value)
{
    if(_tcscmp(name, _T("checked")) == 0) {
        bool checked = _tcscmp(value, _T("true")) == 0;
        ::SendMessage(_hwnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    else return __super::set_attr(name, value);
}

//////////////////////////////////////////////////////////////////////////
void Label::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) {
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
void Group::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) {
    metas.style |= BS_GROUPBOX;
    metas.classname = WC_BUTTON;
}

}
