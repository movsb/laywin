#ifndef __laywin_syscontrols_h__
#define __laywin_syscontrols_h__

#include <map>
#include <vector>
#include <limits.h>
#include <functional>

#include "lw_control.h"
#include "lw_util.h"

namespace laywin{
	struct style_map
	{
		DWORD dwStyle;
		LPCTSTR strStyle;
	};

    struct syscontrol_metas {
        DWORD       style;
        DWORD       exstyle;
        const char* caption;
        const char* classname;
        style_map*  known_styles;
        style_map*  known_ex_styles;
        std::function<void()> after_created;

        syscontrol_metas() {
            style = WS_CHILD | WS_VISIBLE;
            exstyle = 0;
            caption = "";
            classname = "";
            known_styles = nullptr;
            known_ex_styles = nullptr;
            after_created = nullptr;
        }
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
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) = 0;

    private:
        void create_metas(syscontrol_metas& metas, std::map<string, string>& attrs);
	};

	class button : public syscontrol
	{
	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};

	class option : public syscontrol
	{
	public:
		option();

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;

	protected:
		bool _b_has_group;
	};

	class check : public syscontrol
	{
	protected:
        virtual void set_attr(const char* name, const char* value) override;
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};

    
	class label : public syscontrol
	{
	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};

	class group : public syscontrol
	{
	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};

	class edit : public syscontrol
	{
	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};

	class listview : public syscontrol
	{
	public:
		int insert_column(LPCTSTR name, int cx, int i);
		int insert_item(LPCTSTR str, LPARAM param = 0, int i = INT_MAX);
		bool delete_item(int i);
		int set_item(LPCTSTR str, int i, int isub);
		LPARAM get_param(int i, int isub);
		void format_columns(const string& fmt);
        int get_column_count();
		int size() const;

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;

	private:
		string _fmt_column;
	};
}

#endif//__laywin_syscontrols_h__
