#include <assert.h>
#include <windows.h>
#include <CommCtrl.h>
#include <vector>

#include "lw_control.h"
#include "lw_manager.h"
#include "lw_syscontrols.h"

namespace laywin{
	struct style_map
	{
		DWORD dwStyle;
		LPCTSTR strStyle;
	};

	bool map_style(DWORD* dwStyle, style_map* known_styles, std::vector<string>& styles)
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
		return n == styles.size();
	}

	void split_string(std::vector<string>* vec, LPCTSTR str, TCHAR delimiter)
	{
		LPCTSTR p = str;
		string tmp;
		for(;;){
			if(*p){
				if(*p != delimiter){
					tmp += *p;
					p++;
					continue;
				}
				else{
					vec->push_back(tmp);
					tmp = _T("");
					p++;
					continue;
				}
			}
			else{
				if(tmp.size()) vec->push_back(tmp);
				break;
			}
		}
	}

	syscontrol::syscontrol()
		: _dwStyle(WS_CHILD|WS_VISIBLE)
		, _dwExStyle(0)
	{

	}

	void syscontrol::manager_(manager* mgr)
	{
		_mgr = mgr;
		_hwnd = ::CreateWindowEx(_dwExStyle, get_control_class(), _text.c_str(), _dwStyle,
			0, 0, 0, 0, mgr->hwnd(), HMENU(_id), GetModuleHandle(0), NULL);
		assert(_hwnd);
	}

	void syscontrol::init()
	{
		__super::init();
		::SetWindowText(_hwnd, _text.c_str());
	}

	void syscontrol::attribute(LPCTSTR name, LPCTSTR value, bool inited /* = false */)
	{
		if(0);

		else if(_tcscmp(name, _T("style")) == 0){
			std::vector<string> styles;
			split_string(&styles, value, ',');
			set_style(styles);
		}
		else if(_tcscmp(name, _T("exstyle")) == 0){
			std::vector<string> exstyles;
			split_string(&exstyles, value, ',');
			set_style(exstyles, true);
		}
		else __super::attribute(name, value, inited);
	}

	void syscontrol::set_style(std::vector<string>& styles, bool bex /*= false*/)
	{
		static style_map known_styles[] =
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

		static style_map known_ex_styles[] =
		{
			{WS_EX_ACCEPTFILES, _T("acceptfiles")},
			{WS_EX_CLIENTEDGE, _T("clientedge")},
			{WS_EX_STATICEDGE, _T("staticedge")},
			{WS_EX_TOOLWINDOW, _T("toolwindow")},
			{WS_EX_TOPMOST, _T("topmost")},
			{WS_EX_TRANSPARENT, _T("transparent")},
			{0, nullptr}
		};

		auto ks = bex ? &known_ex_styles[0] : &known_styles[0];
		auto& thestyle = bex ? _dwExStyle : _dwStyle;
		if(!map_style(&thestyle, ks, styles)){
			assert(0 && "unknown style detected!");
		}
	}


	LPCTSTR button::get_control_class() const
	{
		return WC_BUTTON;
	}

	void button::set_style(std::vector<string>& styles, bool bex /*= false*/)
	{
		static style_map known_styles[] =
		{
			{BS_MULTILINE, _T("multiline")},
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


	LPCTSTR option::get_control_class() const
	{
		return WC_BUTTON;
	}

	option::option()
		: _b_has_group(false)
	{
		_dwStyle |= BS_AUTORADIOBUTTON;
	}

	void option::set_style(std::vector<string>& styles, bool bex /*= false*/)
	{
		return __super::set_style(styles, bex);
	}


	check::check()
	{
		_b_check = false;
		_dwStyle |= BS_AUTOCHECKBOX;
	}

	void check::init()
	{
		__super::init();
		::SendMessage(_hwnd, BM_SETCHECK, _b_check ? BST_CHECKED : BST_UNCHECKED, 0);
	}

	LPCTSTR check::get_control_class() const
	{
		return WC_BUTTON;
	}

	void check::attribute(LPCTSTR name, LPCTSTR value, bool inited)
	{
		if(_tcscmp(name, _T("checked")) == 0) _b_check = _tcscmp(value, _T("true")) == 0;
		else return __super::attribute(name, value, inited);
	}


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

	void edit::set_style(std::vector<string>& styles, bool bex /* = false */)
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

	void listview::set_style(std::vector<string>& styles, bool bex /* = false */)
	{
		return __super::set_style(styles, bex);
	}

	int listview::insert_item(LPCTSTR str, LPARAM param /*= 0*/, int i /*= */)
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

}
