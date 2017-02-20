#include "core/tw_syscontrols.h"
#include "combobox.h"

namespace taowin {

int ComboboxControl::add_string(const TCHAR * s)
{
    return ComboBox_AddString(_hwnd, s);
}

int ComboboxControl::add_string(const TCHAR * s, void * data)
{
    int i = add_string(s);
    if (i != -1) {
        set_item_data(i, data);
    }

    return i;
}

void ComboboxControl::set_item_data(int i, void * data)
{
    ComboBox_SetItemData(_hwnd, i, data);
}

void * ComboboxControl::get_item_data(int i)
{
    return (void*)ComboBox_GetItemData(_hwnd, i);
}

void * ComboboxControl::get_cur_data()
{
    void* d = nullptr;
    int i = get_cur_sel();

    if (i != -1) {
        d = get_item_data(i);
    }

    return d;
}

int ComboboxControl::get_cur_sel()
{
    return ComboBox_GetCurSel(_hwnd);
}

void ComboboxControl::set_cur_sel(int i)
{
    return (void)ComboBox_SetCurSel(_hwnd, i);
}

int ComboboxControl::get_count()
{
    return ComboBox_GetCount(_hwnd);
}

string ComboboxControl::get_text()
{
    static TCHAR buf[1024];
    int len = ::GetWindowTextLength(_hwnd);
    TCHAR* p = len <= _countof(buf) ? buf : new TCHAR[len + 1];
    *p = _T('\0');
    ::GetWindowText(_hwnd, p, len + 1);
    string s(p);
    if(p != buf) delete[] p;
    return std::move(s);
}

void ComboboxControl::reset_content()
{
    ComboBox_ResetContent(_hwnd);
}

void ComboboxControl::adjust_droplist_width(const std::vector<const TCHAR*>& strs)
{
    HFONT hFont = (HFONT)::SendMessage(_hwnd, WM_GETFONT, 0, 0);
    HDC hDc = ::GetDC(_hwnd);
    HFONT hOldFont = SelectFont(hDc, hFont);

    int padding = 12;
    int max_width = 0;

    for(auto& s : strs) {
        SIZE sz;
        if(::GetTextExtentPoint32(hDc, s, _tcslen(s), &sz)) {
            if(sz.cx > max_width)
                max_width = sz.cx;
        }
    }

    SelectFont(hDc, hOldFont);
    ::ReleaseDC(_hwnd, hDc);

    int vsw = ::GetSystemMetrics(SM_CXVSCROLL);

    ::SendMessage(_hwnd, CB_SETDROPPEDWIDTH, padding + max_width + vsw, 0);
}

void ComboboxControl::drawit(DRAWITEMSTRUCT* dis)
{
    if(!_ondraw) return;
    if(dis->itemID == -1) return;
    
    bool selected = dis->itemAction == ODA_SELECT && dis->itemState & ODS_SELECTED;
    _ondraw(this, dis, dis->itemID, selected);
}

void ComboboxControl::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
{
    static style_map __known_styles[] = {
        {CBS_SIMPLE,    _T("simple")},
        {CBS_DROPDOWN,  _T("dropdown")},
        {CBS_DROPDOWNLIST, _T("droplist")},
        {CBS_OWNERDRAWFIXED, _T("ownerdrawfixed")},
        {CBS_OWNERDRAWVARIABLE, _T("ownerdrawvariable")},
        {CBS_HASSTRINGS, _T("hasstrings")},
        {0, nullptr}
    };

    metas.style |= WS_VSCROLL;
    metas.classname = WC_COMBOBOX;
    metas.known_styles = __known_styles;
    metas.before_creation = [&] {
        if((metas.style & 3) == 0)
            metas.style |= CBS_DROPDOWNLIST;
    };
}


void ComboboxControl::set_attr(const TCHAR* name, const TCHAR* value)
{
    return __super::set_attr(name, value);
}

bool ComboboxControl::filter_notify(int code, NMHDR* hdr, LRESULT* lr)
{
    if(code == CBN_SELCHANGE) {
        if(_on_sel_change) {
            *lr = _on_sel_change(get_cur_sel(), get_cur_data());
            return true;
        }
    }

    return false;
}


}