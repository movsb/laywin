#include <assert.h>
#include <windows.h>
#include <CommCtrl.h>
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
	{

	}

    void syscontrol::create(HWND parent, std::map<string, string>& attrs, resmgr& mgr) {
        syscontrol_metas metas;
        create_metas(metas, attrs);
        _hwnd = ::CreateWindowEx(metas.exstyle, metas.classname, metas.caption, metas.style,
            0, 0, 0, 0, parent, nullptr, nullptr, this);
        assert(_hwnd);
        if(!_hwnd) return;

        if(metas.after_created)
            metas.after_created();

        decltype(attrs.begin()) it;

        it = attrs.find("font");
        HFONT font = it == attrs.end()
            ? mgr.get_font("default")
            : mgr.get_font(it->second.c_str());
        ::SendMessage(_hwnd, WM_SETFONT, WPARAM(font), TRUE);
        if(it != attrs.end())
            attrs.erase(it);

        it = attrs.find("text");
        if(it != attrs.end()) {
            attrs.erase(it);
        }

        for(auto it = attrs.cbegin(); it != attrs.cend(); it++)
            set_attr(it->first.c_str(), it->second.c_str());
    }

    void syscontrol::create_metas(syscontrol_metas& metas, std::map<string, string>& attrs) {
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
            {LVS_SHOWSELALWAYS, _T("showselalways")},
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

        if((it = attrs.find("style")) != attrs.end()) {
            std::vector<string> styles;
            split_string(&styles, it->second.c_str());

            int mapped = 0;
            if(metas.known_styles)
                mapped += __map_style(&metas.style, metas.known_styles, styles);
            mapped += __map_style(&metas.style, &__known_styles[0], styles);

            assert(mapped == styles.size());
            attrs.erase(it);
        }

        if((it = attrs.find("exstyle")) != attrs.end()) {
            std::vector<string> exstyles;
            split_string(&exstyles, it->second.c_str());

            int mapped = 0;
            if(metas.known_ex_styles)
                mapped += __map_style(&metas.exstyle, metas.known_ex_styles, exstyles);
            mapped += __map_style(&metas.exstyle, &__known_ex_styles[0], exstyles);

            assert(mapped == exstyles.size());
            attrs.erase(it);
        }

        if((it = attrs.find("text")) != attrs.end()) {
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

	void check::set_attr(const char* name, const char* value)
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

    //////////////////////////////////////////////////////////////////////////
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
            {LVS_SINGLESEL, "singlesel"},
            {LVS_OWNERDATA, "ownerdata"},
        };

        metas.classname = WC_LISTVIEW;
        metas.style |= LVS_REPORT;
        metas.known_styles = &__known_styles[0];
        metas.after_created = [&]() {
            DWORD dw = ListView_GetExtendedListViewStyle(_hwnd);
            ListView_SetExtendedListViewStyle(_hwnd, dw | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
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

	void listview::format_columns(const string& fmt)
	{

	}

	int listview::set_item(int i, int isub, const char* s)
	{
		LVITEM lvi = {0};
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = isub;
		lvi.pszText = (LPSTR)s; // TODO
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

	bool listview::delete_item(int i)
	{
		return !!ListView_DeleteItem(_hwnd, i);
	}

    int listview::get_column_count() {
        HWND header = ListView_GetHeader(_hwnd);
        return Header_GetItemCount(header);
    }

    int listview::get_selected_count() {
        return ListView_GetSelectedCount(_hwnd);
    }

    int listview::get_next_item(int start, unsigned int flags) {
        return ListView_GetNextItem(_hwnd, start, flags);
    }

    int listview::get_item_count() {
        return ListView_GetItemCount(_hwnd);
    }

    std::string listview::get_item_text(int i, int isub) {
        char buf[1024 * 4];
        LVITEM lvi;
        lvi.iItem = i;
        lvi.iSubItem = isub;
        lvi.pszText = &buf[0];
        lvi.cchTextMax = _countof(buf);
        int len = ::SendMessage(_hwnd, LVM_GETITEMTEXT, WPARAM(i), LPARAM(&lvi));
        return std::string(buf, len);
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
    void listview::set_item_text(int i, int isub, const char* text) {
        ListView_SetItemText(_hwnd, i, isub, (char*)text);  // TODO confirm cannot use const.
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

}
