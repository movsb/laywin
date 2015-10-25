#ifndef __taowin_syscontrols_h__
#define __taowin_syscontrols_h__

#include <map>
#include <vector>
#include <limits.h>
#include <functional>

#include "tw_control.h"
#include "tw_util.h"

namespace taowin{
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
        void create(HWND parent, std::map<string, string>& attrs, resmgr& mgr) override;

	protected:
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
    public:
        void set_text(const char* text) {
            ::SetWindowText(_hwnd, text);
        }

        std::string get_text() {
            char buf[1024];
            char* p = &buf[0];
            int len = ::GetWindowTextLength(_hwnd)+1;
            if(len > _countof(buf))
                p = new char[len];
            p[::GetWindowText(_hwnd, p, len)] = '\0';
            
            std::string s(p);
            if(p != &buf[0])
                delete[] p;
            return std::move(s);
        }
	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};

	class listview : public syscontrol
	{
	public:
        int get_item_count();
        int get_selected_count();
        int get_next_item(int start, unsigned int flags);
        std::string get_item_text(int i, int isub);
        void set_item_text(int i, int isub, const char* text);
        int get_item_data(int i, int isub);
		int insert_column(LPCTSTR name, int cx, int i);
		int insert_item(LPCTSTR str, LPARAM param = 0, int i = INT_MAX);
        inline int insert_item(const std::string& str, LPARAM param = 0, int i = INT_MAX) {
            return insert_item(str.c_str(), param, i);
        }
		bool delete_item(int i);
        bool delete_all_items();
		int set_item(int i, int isub, const char* s);
        inline int set_item(int i, int isub, const std::string& s) {
            return set_item(i, isub, s.c_str());
        }
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

#endif//__taowin_syscontrols_h__
