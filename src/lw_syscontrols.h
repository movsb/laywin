#ifndef __laywin_syscontrols_h__
#define __laywin_syscontrols_h__

#include <map>
#include <vector>
#include <limits.h>

#include "lw_control.h"
#include "lw_util.h"

namespace laywin{
    struct syscontrol_metas {
        DWORD       style;
        DWORD       exstyle;
        const char* caption;
        const char* classname;
    };

	struct style_map
	{
		DWORD dwStyle;
		LPCTSTR strStyle;
	};

	class syscontrol : public control
	{
	public:
		syscontrol();

	public:
		bool set_window_text(LPCTSTR str)
		{
			return !!::SetWindowText(_hwnd, str);
		}

		string get_window_text()
		{
			int len = ::GetWindowTextLength(_hwnd);
			char* p = new char[len + 1];
			p[::GetWindowText(_hwnd, p, len+1)] = '\0';

			string s(p);
			delete[] p;
			return s;
		}

        void create(HWND parent, std::map<string, string>& attrs, resmgr& mgr) override;

	protected:
		virtual void init() override;
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles);
	};

	class button : public syscontrol
	{
	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) override;
	};

	class option : public syscontrol
	{
	public:
		option();

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) override;

	protected:
		bool _b_has_group;
	};

	class check : public syscontrol
	{
	public:
		check();

	protected:
		virtual void init() override;
        virtual void set_attr(const char* name, const char* value) override;
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) override;

	protected:
		bool _b_check;
	};

    /*
	class static_ : public syscontrol
	{
	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) override;
	};

	class group : public syscontrol
	{
	public:
		group();

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) override;
	};

	class edit : public syscontrol
	{
	public:
		edit();

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) override;
	};

	class listview : public syscontrol
	{
	public:
		listview();
		virtual void init() override;
		int insert_column(LPCTSTR name, int cx, int i);
		int insert_item(LPCTSTR str, LPARAM param = 0, int i = INT_MAX);
		bool delete_item(int i);
		int set_item(LPCTSTR str, int i, int isub);
		LPARAM get_param(int i, int isub);
		void format_columns(const string& fmt);
		int size() const;

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs, style_map* known_styles, style_map* known_ex_styles) override;

	private:
		string _fmt_column;
	};
    */
}

#endif//__laywin_syscontrols_h__
