#include <core/tw_syscontrols.h>
#include "header.h"
#include "listview.h"

namespace taowin {

void ListViewControl::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
    static style_map __known_styles[] = {
        {LVS_SINGLESEL, _T("singlesel")},
        {LVS_OWNERDATA, _T("ownerdata")},
        {LVS_SHOWSELALWAYS, _T("showselalways")},
        {0, nullptr}
    };

    static style_map __known_ex_styles[] = {
        {LVS_EX_DOUBLEBUFFER, _T("doublebuffer")},
        {LVS_EX_HEADERDRAGDROP, _T("headerdragdrop")},
        {LVS_EX_INFOTIP, _T("infotip")},
        {0, nullptr}
    };

    metas.classname = WC_LISTVIEW;
    metas.style |= LVS_REPORT;
    metas.known_styles = &__known_styles[0];
    metas.known_ex_styles = &__known_ex_styles[0];
    metas.after_created = [&]() {
        DWORD dw = ListView_GetExtendedListViewStyle(_hwnd);
        ListView_SetExtendedListViewStyle(_hwnd, dw | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
        auto header = get_header();
        assert(header != nullptr);
        _header.attach(header);
    };
}

bool ListViewControl::filter_notify(int code, NMHDR* hdr, LRESULT* lr)
{
    if(code == LVN_GETDISPINFO) {
        auto pdi = reinterpret_cast<NMLVDISPINFO*>(hdr);
        int item = pdi->item.iItem;
        int subitem = pdi->item.iSubItem;
        pdi->item.pszText = const_cast<LPTSTR>(_data->get(item, subitem));
        return true;
    }
    else if(code == NM_DBLCLK) {
        auto nmlv = reinterpret_cast<NMITEMACTIVATE*>(hdr);
        if(_on_dblclick) {
            *lr = _on_dblclick(nmlv->iItem, nmlv->iSubItem);
            return true;
        }
    }
    else if(code == NM_RCLICK) {
        auto nmlv = reinterpret_cast<NMITEMACTIVATE*>(hdr);
        if(_on_dblclick) {
            *lr = _on_right_click(nmlv->iItem, nmlv->iSubItem);
            return true;
        }
    }

    return false;
}

bool ListViewControl::filter_child(HWND child, int code, NMHDR* hdr, LRESULT* lr)
{
    if(child == _header.hwnd()) {
        return _header.filter_notify(code, hdr, lr);
    }

    return false;
}

int ListViewControl::insert_item(LPCTSTR str, LPARAM param, int i)
{
    LVITEM lvi = {0};
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.pszText = (LPTSTR)str;
    lvi.lParam = param;
    lvi.iItem = i;

    return ListView_InsertItem(_hwnd, &lvi);
}

int ListViewControl::insert_column(LPCTSTR name, int cx, int i)
{
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = (LPTSTR)name;
    lvc.cx = cx;
    
    return ListView_InsertColumn(_hwnd, i, &lvc);
}

void ListViewControl::delete_column(int i)
{
    return (void)ListView_DeleteColumn(_hwnd, i);
}

void ListViewControl::format_columns(const string& fmt)
{

}

int ListViewControl::set_item(int i, int isub, const TCHAR* s)
{
    LVITEM lvi = {0};
    lvi.mask = LVIF_TEXT;
    lvi.iItem = i;
    lvi.iSubItem = isub;
    lvi.pszText = (LPTSTR)s; // TODO
    return ListView_SetItem(_hwnd, &lvi);
}

LPARAM ListViewControl::get_param(int i, int isub)
{
    LVITEM lvi = {0};
    lvi.mask = LVIF_PARAM;
    lvi.iItem = i;
    lvi.iSubItem = isub;
    if(ListView_GetItem(_hwnd, &lvi)){
        return lvi.lParam;
    }
    return 0;
}

int ListViewControl::size() const
{
    return ListView_GetItemCount(_hwnd);
}

bool ListViewControl::get_selected_items(std::vector<int>* items)
{
    int i = -1;
    while ((i = get_next_item(i, LVNI_SELECTED)) != -1)
        items->push_back(i);
    return !items->empty();
}

void ListViewControl::set_item_state(int i, int mask, int state)
{
    ListView_SetItemState(_hwnd, i, state, mask);
}

void ListViewControl::ensure_visible(int i)
{
    ListView_EnsureVisible(_hwnd, i, FALSE);
}

void ListViewControl::show_header(int state)
{
    auto style = ::GetWindowLongPtr(_hwnd, GWL_STYLE);
    bool hiding = !!(style & LVS_NOCOLUMNHEADER);

    if (state == -1) state = hiding ? 1 : 0;

    switch(state)
    {
    case 1:  style &= ~LVS_NOCOLUMNHEADER; break;
    case 0:  style |= LVS_NOCOLUMNHEADER; break;
    }

    ::SetWindowLongPtr(_hwnd, GWL_STYLE, style);
}

int ListViewControl::get_top_index()
{
    return ListView_GetTopIndex(_hwnd);
}

bool ListViewControl::get_item_position(int i, POINT * ppt)
{
    return !!ListView_GetItemPosition(_hwnd, i, ppt);
}

void ListViewControl::scroll(int dx, int dy)
{
    ListView_Scroll(_hwnd, dx, dy);
}

int ListViewControl::find_item(int start, const LVFINDINFO * lvfi)
{
    return ListView_FindItem(_hwnd, start, lvfi);
}

int ListViewControl::get_column_order(int n, int * a)
{
    return ListView_GetColumnOrderArray(_hwnd, n, a);
}

void ListViewControl::set_column_order(int n, int * a)
{
    ListView_SetColumnOrderArray(_hwnd, n, a);
}

int ListViewControl::subitem_hittest(LVHITTESTINFO* pht)
{
    return ListView_SubItemHitTest(_hwnd, pht);
}

int ListViewControl::get_subitem_rect(int item, int subitem, RECT* rc, int code)
{
    return ListView_GetSubItemRect(_hwnd, item, subitem, code, rc);
}

bool ListViewControl::showtip_needed(const POINT& pt, const TCHAR** s)
{
    LVHITTESTINFO hti;
    hti.pt = pt;

    if(subitem_hittest(&hti) == -1)
        return false;

    NMLVDISPINFO dispinfo;
    dispinfo.hdr.hwndFrom = _hwnd;
    dispinfo.hdr.idFrom = ::GetWindowLongPtr(_hwnd, GWL_ID);
    dispinfo.hdr.code = LVN_GETDISPINFO;
    dispinfo.item.iItem = hti.iItem;
    dispinfo.item.iSubItem = hti.iSubItem;
    dispinfo.item.pszText = nullptr;

    ::SendMessage(::GetParent(_hwnd), WM_NOTIFY, 0, LPARAM(&dispinfo));

    if(!dispinfo.item.pszText || !*dispinfo.item.pszText)
        return false;

    bool need_tip = false;

    auto text = dispinfo.item.pszText;

    // 1) 有换行符
    if(!need_tip) {
        if(_tcschr(text, _T('\n'))) {
            need_tip = true;
        }
    }

    // 不初始化会报潜在使用了未初始化的变量（但实际上不可能）
    int text_width = 0;

    // http://stackoverflow.com/questions/51109/win32-list-view-control-subitem-padding-for-custom-drawn-subitems
    int text_padding = Header_GetBitmapMargin(get_header()) * 2;

    // 2) 文本宽度超出列宽
    if(!need_tip) {
        HDC hdc = ::GetDC(_hwnd);
        HFONT hFont = (HFONT)::SendMessage(_hwnd, WM_GETFONT, 0, 0);
        HFONT hOldFont = SelectFont(hdc, hFont);

        SIZE szText = {0};
        if(::GetTextExtentPoint32(hdc, text, _tcslen(text), &szText)) {
            int col_width = get_column_width(hti.iSubItem);
            text_width = szText.cx + text_padding;

            if(text_width > col_width) {
                need_tip = true;
            }
        }

        SelectFont(hdc, hOldFont);

        ::ReleaseDC(_hwnd, hdc);
    }

    // 3) 列没有完整地显示出来
    // 包括：滚动条出现、位于屏幕外
    if(!need_tip) {
        taowin::Rect rcSubItem, rcListView;
        ::GetClientRect(_hwnd, &rcListView);

        if(get_subitem_rect(hti.iItem, hti.iSubItem, &rcSubItem)) {
            // 换算成 SubItem 的实际尺寸
            rcSubItem.right = rcSubItem.left + text_width;

            if(rcSubItem.left < rcListView.left || rcSubItem.right > rcListView.right) {
                need_tip = true;
            }

            if(!need_tip) {
                // 列的屏幕位置（B）
                taowin::Rect rc(rcSubItem);
                ::ClientToScreen(_hwnd, reinterpret_cast<POINT*>(&rc.left));
                ::ClientToScreen(_hwnd, reinterpret_cast<POINT*>(&rc.right));

                // 屏幕位置（A）
                taowin::Rect rcScreen;
                {
                    POINT pt;
                    ::GetCursorPos(&pt);

                    HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
                    MONITORINFO info = {sizeof(info)};

                    if(hMonitor && ::GetMonitorInfo(hMonitor, &info)) {
                        rcScreen = info.rcWork;
                    }
                    else {
                        rcScreen = {0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN)};
                    }
                }

                // 如果 A & B != B
                if(rcScreen.join(rc) != rc) {
                    need_tip = true;
                }
            }
        }
    }

    *s = text;
    return need_tip;
}

bool ListViewControl::delete_item(int i)
{
    return !!ListView_DeleteItem(_hwnd, i);
}

int ListViewControl::get_column_count() {
    HWND header = ListView_GetHeader(_hwnd);
    return Header_GetItemCount(header);
}

void ListViewControl::update_columns()
{
    _columns.update();
    _columns.each_showing([&](int i, Column& c) {
        insert_column(c.name.c_str(), c.width, i);
    });
}

void ListViewControl::set_column_width(int i, int cx)
{
    return (void)ListView_SetColumnWidth(_hwnd, i, cx);
}

int ListViewControl::get_column_width(int i)
{
    return ListView_GetColumnWidth(_hwnd, i);
}

HWND ListViewControl::get_header()
{
    return ListView_GetHeader(_hwnd);
}

int ListViewControl::get_selected_count()
{
    return ListView_GetSelectedCount(_hwnd);
}

int ListViewControl::get_next_item(int start, unsigned int flags) {
    return ListView_GetNextItem(_hwnd, start, flags);
}

unsigned int ListViewControl::get_item_state(int i, unsigned int flag)
{
    return ListView_GetItemState(_hwnd, i, flag);
}

int ListViewControl::get_item_count() {
    return ListView_GetItemCount(_hwnd);
}

string ListViewControl::get_item_text(int i, int isub) {
    TCHAR buf[1024 * 4];
    LVITEM lvi;
    lvi.iItem = i;
    lvi.iSubItem = isub;
    lvi.pszText = &buf[0];
    lvi.cchTextMax = _countof(buf);
    int len = ::SendMessage(_hwnd, LVM_GETITEMTEXT, WPARAM(i), LPARAM(&lvi));
    return string(buf, len);
}

int ListViewControl::get_item_data(int i, int isub) {
    LVITEM li;
    li.iItem = i;
    li.iSubItem = isub;
    li.mask = LVIF_PARAM;
    ListView_GetItem(_hwnd, &li);
    return (int)li.lParam;
}

// I used macro, so it return nothing(msdn says so).
void ListViewControl::set_item_text(int i, int isub, const TCHAR* text) {
    ListView_SetItemText(_hwnd, i, isub, (TCHAR*)text);  // TODO confirm cannot use const.
}

bool ListViewControl::delete_all_items() {
    return !!ListView_DeleteAllItems(_hwnd);
}

bool ListViewControl::set_item_count(int count, int flags) {
    return !!ListView_SetItemCountEx(_hwnd, count, flags);
}

bool ListViewControl::redraw_items(int first, int last) {
    return !!ListView_RedrawItems(_hwnd, first, last);
}

void ListViewControl::ColumnManager::update()
{
    _available_indices.clear();
    _showing_indices.clear();

    int i = 0;

    for(auto& c : _columns) {
        if(c.valid) {
            _available_indices.emplace_back(i);

            if(c.show) {
                _showing_indices.emplace_back(i);
            }
        }

        i++;
    }
}

void ListViewControl::ColumnManager::each_all(std::function<void(int i, Column& c)> fn)
{
    int i = 0;
    for(auto& c : _columns) {
        fn(i++, c);
    }
}

void ListViewControl::ColumnManager::each_avail(std::function<void(int i, Column& c)> fn)
{
    int i = 0;
    for(auto& c : _available_indices) {
        fn(i++, _columns[c]);
    }
}

void ListViewControl::ColumnManager::each_showing(std::function<void(int i, Column& c)> fn)
{
    int i = 0;
    for(auto& c : _showing_indices) {
        fn(i++, _columns[c]);
    }
}

void ListViewControl::ColumnManager::show(int available_index, int* listview_index)
{
    if(available_index < 0 || available_index >= (int)_available_indices.size())
        return;

    auto ai = _available_indices[available_index];
    auto& c = _columns[ai];

    if(!c.valid || c.show)
        return;

    int pos = 0;
    for(auto p : _showing_indices) {
        if(p > ai)
            break;
        pos++;
    }

    *listview_index = pos;
    _showing_indices.emplace(_showing_indices.cbegin() + pos, ai);
}

void ListViewControl::ColumnManager::hide(bool is_listview_index, int index, int* listview_delete_index)
{
    int ai = is_listview_index ? _showing_indices[index] : _available_indices[index];

    int show_index = 0;

    for(auto p : _showing_indices) {
        if(p == ai) {
            break;
        }
        show_index++;
    }

    if(!is_listview_index) *listview_delete_index = show_index;
    _showing_indices.erase(_showing_indices.cbegin() + show_index);
}



}