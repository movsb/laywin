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
        DWORD           style;
        DWORD           exstyle;
        const TCHAR*    caption;
        const TCHAR*     classname;
        style_map*      known_styles;
        style_map*      known_ex_styles;
        std::function<void()> after_created;

        syscontrol_metas() {
            style = WS_CHILD | WS_VISIBLE;
            exstyle = 0;
            caption = _T("");
            classname = _T("");
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
    public:
        void set_text(const TCHAR* text) {
            ::SetWindowText(_hwnd, text);
        }

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
    public:
        bool get_chekc();

        virtual void set_attr(const TCHAR* name, const TCHAR* value) override;

	protected:
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

    class combobox : public syscontrol
    {
    public:
        int add_string(const TCHAR* s);
        int add_string(const TCHAR* s, void* data);
        void set_item_data(int i, void* data);
        void* get_item_data(int i);
        void* get_cur_data();
        int get_cur_sel();
        void set_cur_sel(int i);
        int get_count();
        void reset_content();
        
    protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
    };

	class edit : public syscontrol
	{
    public:
        void set_text(const TCHAR* text) {
            ::SetWindowText(_hwnd, text);
        }

        string get_text() {
            TCHAR buf[1024];
            TCHAR* p = &buf[0];
            int len = ::GetWindowTextLength(_hwnd)+1;
            if(len > _countof(buf))
                p = new TCHAR[len];
            p[::GetWindowText(_hwnd, p, len)] = _T('\0');
            
            string s(p);
            if(p != &buf[0])
                delete[] p;
            return std::move(s);
        }

        void set_sel(int start, int end);

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};

	class listview : public syscontrol
	{
	public:
        bool redraw_items(int first, int last);
        bool set_item_count(int count, int flags);
        int get_item_count();
        int get_selected_count();
        int get_next_item(int start, unsigned int flags);
        string get_item_text(int i, int isub);
        void set_item_text(int i, int isub, const TCHAR* text);
        int get_item_data(int i, int isub);
		int insert_column(LPCTSTR name, int cx, int i);
		int insert_item(LPCTSTR str, LPARAM param = 0, int i = INT_MAX);
        inline int insert_item(const string& str, LPARAM param = 0, int i = INT_MAX) {
            return insert_item(str.c_str(), param, i);
        }
		bool delete_item(int i);
        bool delete_all_items();
		int set_item(int i, int isub, const TCHAR* s);
        inline int set_item(int i, int isub, const string& s) {
            return set_item(i, isub, s.c_str());
        }
		LPARAM get_param(int i, int isub);
		void format_columns(const string& fmt);
        int get_column_count();
        void set_column_width(int i, int cx);
        HWND get_header();
		int size() const;
        bool get_selected_items(std::vector<int>* items);
        void set_item_state(int i, int mask, int state);
        void ensure_visible(int i);
        void show_header(int state); // 1:show, 0, hide, -1: toggle
        int get_top_index();
        bool get_item_position(int i, POINT* ppt);
        void scroll(int dx, int dy);
        int find_item(int start, const LVFINDINFO* lvfi);

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;

	private:
		string _fmt_column;
	};
}

#endif//__taowin_syscontrols_h__
