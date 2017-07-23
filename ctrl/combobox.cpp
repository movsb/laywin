#include <taowin/core/tw_syscontrols.h>
#include "combobox.h"

namespace taowin {

ComboBox::ComboBox()
:_source(nullptr)
{

}

ComboBox::~ComboBox()
{

}

int ComboBox::add_string(const TCHAR * s)
{
    return ComboBox_AddString(_hwnd, s);
}

int ComboBox::add_string(const TCHAR * s, void * data)
{
    int i = add_string(s);
    if (i != -1) {
        set_item_data(i, data);
    }

    return i;
}

void ComboBox::set_item_data(int i, void * data)
{
    ComboBox_SetItemData(_hwnd, i, data);
}

void * ComboBox::get_item_data(int i)
{
    return (void*)ComboBox_GetItemData(_hwnd, i);
}

void * ComboBox::get_cur_data()
{
    void* d = nullptr;
    int i = get_cur_sel();

    if (i != -1) {
        d = get_item_data(i);
    }
    else {
        LogWrn(_T("当前没有选中项，返回 NULL"));
    }

    return d;
}

int ComboBox::get_cur_sel()
{
    return ComboBox_GetCurSel(_hwnd);
}

void ComboBox::set_cur_sel(int i)
{
    auto n = get_count();
    if (i == -1 || (i >= 0 && i < n)) {
        return (void)ComboBox_SetCurSel(_hwnd, i);
    }
    else {
        LogWrn(_T("无效当前索引：%d"), i);
    }
}

void ComboBox::set_cur_sel(void* p)
{
    bool set = false;

    if(_source == nullptr) {
        auto n = get_count();
        for(decltype(n) i = 0; i < n; ++i) {
            auto v = get_item_data(i);
            if(v == p) {
                set_cur_sel(i);
                set = true;
                LogLog(_T("选中第 %d 项"), i);
                break;
            }
        }
    }
    else {
        for(int i = 0, n = _source->Size(); i < n; ++i) {
            const TCHAR* text;
            void* tag;
            _source->GetAt(i, &text, &tag);
            if(tag == p) {
                set_cur_sel(i);
                set = true;
                LogLog(_T("选中第 %d 项"), i);
                break;
            }
        }
    }

    if(!set) {
        LogWrn(_T("未能设置到指定数据项，因为没找到"));
    }
}

int ComboBox::get_count()
{
    return ComboBox_GetCount(_hwnd);
}

string ComboBox::get_text()
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

void ComboBox::clear()
{
    ComboBox_ResetContent(_hwnd);
}

void ComboBox::adjust_droplist_width()
{
    if(_source == nullptr) {
        LogWrn(_T("没有数据源可用。"));
        return;
    }

    HFONT hFont = (HFONT)::SendMessage(_hwnd, WM_GETFONT, 0, 0);
    HDC hDc = ::GetDC(_hwnd);
    HFONT hOldFont = SelectFont(hDc, hFont);

    int padding = 6;
    int max_width = 0;

    for(int i = 0, n = _source->Size(); i < n; ++i) {
        SIZE sz;
        const TCHAR* text;
        void* tag;

        _source->GetAt(i, &text, &tag);

        if(::GetTextExtentPoint32(hDc, text, _tcslen(text), &sz)) {
            if(sz.cx > max_width) {
                max_width = sz.cx;
            }
        }
    }

    LogLog(_T("文本最大宽度值：%d"), max_width);

    SelectFont(hDc, hOldFont);
    ::ReleaseDC(_hwnd, hDc);

    int vsw = ::GetSystemMetrics(SM_CXVSCROLL);
    int width = padding * 2 + max_width + vsw;

    LogLog(_T("最终设置宽度值：%d"), width);

    ::SendMessage(_hwnd, CB_SETDROPPEDWIDTH, width, 0);
}

void ComboBox::drawit(DRAWITEMSTRUCT* dis)
{
    if(!_ondraw) return;
    if(dis->itemID == -1) return;
    
    bool selected = dis->itemAction == ODA_SELECT && dis->itemState & ODS_SELECTED;
    _ondraw(this, dis, dis->itemID, selected);
}

void ComboBox::set_source(IDataSource* source)
{
    _source = source;

    reload();
}

void ComboBox::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs)
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


void ComboBox::set_attr(const TCHAR* name, const TCHAR* value)
{
    return __super::set_attr(name, value);
}

bool ComboBox::filter_notify(int code, NMHDR* hdr, LRESULT* lr)
{
    if(code == CBN_SELCHANGE) {
        if(_on_sel_change) {
            *lr = _on_sel_change(get_cur_sel(), get_cur_data());
            return true;
        }
    }

    return false;
}


void ComboBox::reload(bool keep_selection)
{
    int back_index = -1;
    void* back_tag = nullptr;

    if(keep_selection) {
        back_index = get_cur_sel();
        back_tag = get_cur_data();
        LogLog(_T("备份当前选中项：index=%d, tag=%p"), back_index, back_tag);
    }

    clear();

    if (_source == nullptr) {
        LogWrn(_T("no data to be load"));
        return;
    }

    auto n = _source->Size();
    LogLog(_T("size = %u"), n);

    for (decltype(n) i = 0; i < n; ++i) {
        TCHAR const* text = _T("");
        void* tag = nullptr;

        _source->GetAt(i, &text, &tag);

        if (text == nullptr) {
            LogWrn(_T("index=%d: text == nullptr"), i);
            text = _T("");
        }

        add_string(text, tag);
    }

    if(keep_selection) {
        LogLog(_T("恢复选中数据项：index=%d, tag=%p"), back_index, back_tag);
        if(back_index== -1) {
            set_cur_sel(back_index);
        }
        else {
            set_cur_sel(back_tag);
        }
    }

    // 默认自动更新下拉宽度
    adjust_droplist_width();
}

}
