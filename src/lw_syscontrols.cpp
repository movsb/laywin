#include <assert.h>
#include <windows.h>
#include <CommCtrl.h>
#include <vector>

#include "lw_control.h"
#include "lw_resmgr.h"
#include "lw_syscontrols.h"

namespace laywin{

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
            else{
                n++;
            }
        }
        return n;
    }

	syscontrol::syscontrol()
	{

	}

	void syscontrol::init()
	{
		__super::init();
	}

    void syscontrol::create(HWND parent, std::map<string, string>& attrs, resmgr& mgr) {
        syscontrol_metas metas;
        get_metas(metas, attrs, nullptr, nullptr);
        _hwnd = ::CreateWindowEx(metas.exstyle, metas.classname, metas.caption, metas.style,
            0, 0, 0, 0, parent, nullptr, nullptr, this);
        assert(_hwnd);

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
            ::SetWindowText(_hwnd, it->second.c_str());
            attrs.erase(it);
        }

        for(auto it = attrs.cbegin(); it != attrs.cend(); it++)
            set_attr(it->first.c_str(), it->second.c_str());
    }

    void syscontrol::get_metas(syscontrol_metas& metas, std::map<string,string>& attrs, 
        style_map* known_styles, style_map* known_ex_styles) 
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

        metas.style = WS_CHILD | WS_VISIBLE;
        metas.exstyle = 0;

        decltype(attrs.begin()) it;

        if((it = attrs.find("style")) != attrs.end()) {
            std::vector<string> styles;
            split_string(&styles, it->second.c_str());

            int mapped = __map_style(&metas.style, &__known_styles[0], styles);
            if(known_styles)
                mapped += __map_style(&metas.style, known_styles, styles);

            assert(mapped == styles.size());
            attrs.erase(it);
        }

        if((it = attrs.find("exstyle")) != attrs.end()) {
            std::vector<string> exstyles;
            split_string(&exstyles, it->second.c_str());

            int mapped = __map_style(&metas.exstyle, &__known_ex_styles[0], exstyles);
            if(known_ex_styles)
                mapped += __map_style(&metas.exstyle, known_ex_styles, exstyles);

            assert(mapped == exstyles.size());
            attrs.erase(it);
        }

        if((it = attrs.find("text")) != attrs.end()) {
            auto& text = it->second;
            metas.caption = text.c_str();   // TODO ALERT text is not local!
        }
    }

    //////////////////////////////////////////////////////////////////////////
    void button::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) {
        static style_map __known_styles[] =
        {
            {BS_MULTILINE, _T("multiline")},
            {0, nullptr}
        };

        static style_map __known_ex_styles[] =
        {
            {0, nullptr}
        };

        __super::get_metas(metas, attrs, __known_styles, __known_ex_styles);
        metas.classname = WC_BUTTON;
    }

    //////////////////////////////////////////////////////////////////////////
	option::option()
		: _b_has_group(false)
	{
	}

    void option::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) {
        __super::get_metas(metas, attrs, known_styles, known_ex_styles);
        metas.classname = WC_BUTTON;
        metas.style |= BS_AUTORADIOBUTTON;
    }

    //////////////////////////////////////////////////////////////////////////
	check::check()
	{
	}

	void check::init()
	{
	}

    void check::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) {
        __super::get_metas(metas, attrs, known_styles, known_ex_styles);
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

    /*
    //////////////////////////////////////////////////////////////////////////
	LPCTSTR static_::get_control_class() const
	{
		return WC_STATIC;
	}


	group::group()
	{
		_dwStyle |= BS_GROUPBOX;
	}

	LPCTSTR group::get_control_class() const
	{
		return WC_BUTTON;
	}


	edit::edit()
	{
		_dwStyle |= ES_AUTOHSCROLL | ES_AUTOVSCROLL;
	}

	LPCTSTR edit::get_control_class() const
	{
		return WC_EDIT;
	}

	void edit::set_style(std::vector<string>& styles, bool bex)
	{
		static style_map known_styles[] =
		{
			{ES_CENTER, _T("center")},
			{ES_MULTILINE, _T("multiline")},
			{ES_NOHIDESEL, _T("nohidesel")},
			{ES_NUMBER, _T("number")},
			{ES_READONLY, _T("readonly")},
			{ES_WANTRETURN, _T("wantreturn")},
			{0, nullptr}
		};

		static style_map known_ex_styles[] =
		{
			{0, nullptr}
		};

		auto ks = bex ? &known_ex_styles[0] : &known_styles[0];
		auto& thestyle = bex ? _dwExStyle : _dwStyle;
		if(!map_style(&thestyle, ks, styles)){
			return __super::set_style(styles, bex);
		}
	}


	listview::listview()
	{
		_dwStyle |= LVS_REPORT;
	}

	LPCTSTR listview::get_control_class() const
	{
		return WC_LISTVIEW;
	}

	void listview::set_style(std::vector<string>& styles, bool bex)
	{
		return __super::set_style(styles, bex);
	}

	int listview::insert_item(LPCTSTR str, LPARAM param)
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

	void listview::init()
	{
		__super::init();
		DWORD dw = ListView_GetExtendedListViewStyle(_hwnd);
		ListView_SetExtendedListViewStyle(_hwnd, dw | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	}

	void listview::format_columns(const string& fmt)
	{

	}

	int listview::set_item(LPCTSTR str, int i, int isub)
	{
		LVITEM lvi = {0};
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = isub;
		lvi.pszText = (LPSTR)str;
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
    */
}
