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

	syscontrol::syscontrol()
        : _old_wnd_proc(0)
        , _owner(0)
	{

	}

    void syscontrol::create(HWND parent, std::map<string, string>& attrs, resmgr& mgr) {
        syscontrol_metas metas;
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

    unsigned int syscontrol::get_ctrl_id() const
    {
        return ::GetWindowLongPtr(_hwnd, GWL_ID);
    }

    void syscontrol::create_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
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


    //////////////////////////////////////////////////////////////////////////
    void button::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
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

    //////////////////////////////////////////////////////////////////////////
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

    int combobox::add_string(const TCHAR * s)
    {
        return ComboBox_AddString(_hwnd, s);
    }

    int combobox::add_string(const TCHAR * s, void * data)
    {
        int i = add_string(s);
        if (i != -1) {
            set_item_data(i, data);
        }

        return i;
    }

    void combobox::set_item_data(int i, void * data)
    {
        ComboBox_SetItemData(_hwnd, i, data);
    }

    void * combobox::get_item_data(int i)
    {
        return (void*)ComboBox_GetItemData(_hwnd, i);
    }

    void * combobox::get_cur_data()
    {
        void* d = nullptr;
        int i = get_cur_sel();

        if (i != -1) {
            d = get_item_data(i);
        }

        return d;
    }

    int combobox::get_cur_sel()
    {
        return ComboBox_GetCurSel(_hwnd);
    }

    void combobox::set_cur_sel(int i)
    {
        return (void)ComboBox_SetCurSel(_hwnd, i);
    }

    int combobox::get_count()
    {
        return ComboBox_GetCount(_hwnd);
    }

    string combobox::get_text()
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

    void combobox::reset_content()
    {
        ComboBox_ResetContent(_hwnd);
    }

    void combobox::adjust_droplist_width(const std::vector<const TCHAR*>& strs)
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

    void combobox::drawit(DRAWITEMSTRUCT* dis)
    {
        if(!_ondraw) return;
        if(dis->itemID == -1) return;
        
        bool selected = dis->itemAction == ODA_SELECT && dis->itemState & ODS_SELECTED;
        _ondraw(this, dis, dis->itemID, selected);
    }

    void combobox::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
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


    void combobox::set_attr(const TCHAR* name, const TCHAR* value)
    {
		return __super::set_attr(name, value);
	}

    //////////////////////////////////////////////////////////////////////////
    void edit::set_sel(int start, int end)
    {
        Edit_SetSel(_hwnd, start, end);
    }

    void edit::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
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

    //////////////////////////////////////////////////////////////////////////
    void listview::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
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
        };
    }

	int listview::insert_item(LPCTSTR str, LPARAM param, int i)
	{
		LVITEM lvi = {0};
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = (LPTSTR)str;
		lvi.lParam = param;
		lvi.iItem = i;

		return ListView_InsertItem(_hwnd, &lvi);
	}

	int listview::insert_column(LPCTSTR name, int cx, int i)
	{
		LVCOLUMN lvc = {0};
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		lvc.pszText = (LPTSTR)name;
		lvc.cx = cx;
		
		return ListView_InsertColumn(_hwnd, i, &lvc);
	}

    void listview::delete_column(int i)
    {
        return (void)ListView_DeleteColumn(_hwnd, i);
    }

    void listview::format_columns(const string& fmt)
	{

	}

	int listview::set_item(int i, int isub, const TCHAR* s)
	{
		LVITEM lvi = {0};
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = isub;
		lvi.pszText = (LPTSTR)s; // TODO
		return ListView_SetItem(_hwnd, &lvi);
	}

	LPARAM listview::get_param(int i, int isub)
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

	int listview::size() const
	{
		return ListView_GetItemCount(_hwnd);
	}

    bool listview::get_selected_items(std::vector<int>* items)
    {
        int i = -1;
        while ((i = get_next_item(i, LVNI_SELECTED)) != -1)
            items->push_back(i);
        return !items->empty();
    }

    void listview::set_item_state(int i, int mask, int state)
    {
        ListView_SetItemState(_hwnd, i, state, mask);
    }

    void listview::ensure_visible(int i)
    {
        ListView_EnsureVisible(_hwnd, i, FALSE);
    }

    void listview::show_header(int state)
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

    int listview::get_top_index()
    {
        return ListView_GetTopIndex(_hwnd);
    }

    bool listview::get_item_position(int i, POINT * ppt)
    {
        return !!ListView_GetItemPosition(_hwnd, i, ppt);
    }

    void listview::scroll(int dx, int dy)
    {
        ListView_Scroll(_hwnd, dx, dy);
    }

    int listview::find_item(int start, const LVFINDINFO * lvfi)
    {
        return ListView_FindItem(_hwnd, start, lvfi);
    }

	int listview::get_column_order(int n, int * a)
	{
		return ListView_GetColumnOrderArray(_hwnd, n, a);
	}

    void listview::set_column_order(int n, int * a)
    {
        ListView_SetColumnOrderArray(_hwnd, n, a);
    }

    int listview::subitem_hittest(LVHITTESTINFO* pht)
    {
        return ListView_SubItemHitTest(_hwnd, pht);
    }

    int listview::get_subitem_rect(int item, int subitem, RECT* rc, int code)
    {
        return ListView_GetSubItemRect(_hwnd, item, subitem, code, rc);
    }

    bool listview::showtip_needed(const POINT& pt, const TCHAR** s)
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
        constexpr int text_padding = 20;

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

    bool listview::delete_item(int i)
	{
		return !!ListView_DeleteItem(_hwnd, i);
	}

    int listview::get_column_count() {
        HWND header = ListView_GetHeader(_hwnd);
        return Header_GetItemCount(header);
    }

    void listview::set_column_width(int i, int cx)
    {
        return (void)ListView_SetColumnWidth(_hwnd, i, cx);
    }

    int listview::get_column_width(int i)
    {
        return ListView_GetColumnWidth(_hwnd, i);
    }

    HWND listview::get_header()
    {
        return ListView_GetHeader(_hwnd);
    }

    int listview::get_selected_count()
    {
        return ListView_GetSelectedCount(_hwnd);
    }

    int listview::get_next_item(int start, unsigned int flags) {
        return ListView_GetNextItem(_hwnd, start, flags);
    }

    unsigned int listview::get_item_state(int i, unsigned int flag)
    {
        return ListView_GetItemState(_hwnd, i, flag);
    }

    int listview::get_item_count() {
        return ListView_GetItemCount(_hwnd);
    }

    string listview::get_item_text(int i, int isub) {
        TCHAR buf[1024 * 4];
        LVITEM lvi;
        lvi.iItem = i;
        lvi.iSubItem = isub;
        lvi.pszText = &buf[0];
        lvi.cchTextMax = _countof(buf);
        int len = ::SendMessage(_hwnd, LVM_GETITEMTEXT, WPARAM(i), LPARAM(&lvi));
        return string(buf, len);
    }

    int listview::get_item_data(int i, int isub) {
        LVITEM li;
        li.iItem = i;
        li.iSubItem = isub;
        li.mask = LVIF_PARAM;
        ListView_GetItem(_hwnd, &li);
        return (int)li.lParam;
    }

    // I used macro, so it return nothing(msdn says so).
    void listview::set_item_text(int i, int isub, const TCHAR* text) {
        ListView_SetItemText(_hwnd, i, isub, (TCHAR*)text);  // TODO confirm cannot use const.
    }

    bool listview::delete_all_items() {
        return !!ListView_DeleteAllItems(_hwnd);
    }

    bool listview::set_item_count(int count, int flags) {
        return !!ListView_SetItemCountEx(_hwnd, count, flags);
    }

    bool listview::redraw_items(int first, int last) {
        return !!ListView_RedrawItems(_hwnd, first, last);
    }

    //////////////////////////////////////////////////////////////////////////

    int tabctrl::insert_item(int i, const TCHAR* s, const void* p)
    {
        TCITEM item = {0};
        item.mask = TCIF_TEXT | TCIF_PARAM;
        item.pszText = const_cast<TCHAR*>(s);
        item.lParam = (LPARAM)p;

        return TabCtrl_InsertItem(_hwnd, i, &item);
    }

    void tabctrl::adjust_rect(bool large, RECT * rc)
    {
        TabCtrl_AdjustRect(_hwnd, !!large, rc);
    }

    void tabctrl::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
    {
        metas.classname = WC_TABCONTROL;
    }

}
