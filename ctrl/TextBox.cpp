#include <taowin/core/tw_syscontrols.h>
#include "TextBox.h"

namespace taowin {

TextBox::TextBox()
: _crTextColor(RGB(0,0,0))
, _crBackgroundColor(RGB(255,255,255))
, _hBrushBackground(nullptr)
{
    _hBrushBackground = ::CreateSolidBrush(_crBackgroundColor);
}

TextBox::~TextBox()
{
    if (_hBrushBackground != nullptr) {
        ::DeleteBrush(_hBrushBackground);
        _hBrushBackground = nullptr;
    }
}

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

HBRUSH TextBox::handle_ctlcoloredit(HDC hdc)
{
    ::SetTextColor(hdc, _crTextColor);
    ::SetBkMode(hdc, TRANSPARENT);
    ::SetBkColor(hdc, _crBackgroundColor);
    return _hBrushBackground;
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

void TextBox::set_attr(const TCHAR* name, const TCHAR* value)
{
    if (is_attr(_T("foreground"))) {
        int r, g, b;
        if (::_stscanf(value, _T("#%02X%02X%02X"), &r, &g, &b) == 3){
            _crTextColor = RGB(r, g, b);
        }
    }
    else if (is_attr(_T("background"))) {
        int r, g, b;
        if (::_stscanf(value, _T("#%02X%02X%02X"), &r, &g, &b) == 3){
            _crBackgroundColor = RGB(r, g, b);
            _hBrushBackground = ::CreateSolidBrush(_crBackgroundColor);
        }
    }
    else {
        return __super::set_attr(name, value);
    }
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
