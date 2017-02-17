#ifndef __taowin_syscontrols_h__
#define __taowin_syscontrols_h__

#include <map>
#include <vector>
#include <limits.h>
#include <functional>

#include "../etwlogger.h"

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
        std::function<void()> before_creation;

        syscontrol_metas() {
            style = WS_CHILD | WS_VISIBLE;
            exstyle = 0;
            caption = _T("");
            classname = _T("");
            known_styles = nullptr;
            known_ex_styles = nullptr;
        }
    };

    class window_creator;

	class syscontrol : public control
	{
        friend class window_creator;

	public:
		syscontrol();

	public:
        void create(HWND parent, std::map<string, string>& attrs, resmgr& mgr) override;
        unsigned int get_ctrl_id() const;

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) = 0;
        virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) { return false; }

    private:
        void create_metas(syscontrol_metas& metas, std::map<string, string>& attrs);

    private:
        window_creator* _owner;
        WNDPROC _old_wnd_proc;
	};

	class button : public syscontrol
	{
    public:
        typedef std::function<void()> OnMouseEvent;

    public:
        void set_text(const TCHAR* text) {
            ::SetWindowText(_hwnd, text);
        }

        void on_click(OnMouseEvent callback) { _on_click = callback; }

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
        virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) override;

    protected:
        OnMouseEvent    _on_click;
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
    public:
        void set_text(const TCHAR* s)
        {
            ::SetWindowText(_hwnd, s);
        }

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
        typedef std::function<void(combobox* that, DRAWITEMSTRUCT* dis, int i, bool selected)> OnDraw;

    public:
        int add_string(const TCHAR* s);
        int add_string(const TCHAR* s, void* data);
        void set_item_data(int i, void* data);
        void* get_item_data(int i);
        void* get_cur_data();
        int get_cur_sel();
        void set_cur_sel(int i);
        int get_count();
        string get_text();
        void reset_content();
        void adjust_droplist_width(const std::vector<const TCHAR*>& strs);
        void set_ondraw(OnDraw fn) { _ondraw = fn; }
        void drawit(DRAWITEMSTRUCT* dis);
        
    protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
        virtual void set_attr(const TCHAR* name, const TCHAR* value) override;

    private:
        OnDraw _ondraw;
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
        void replace_sel(const TCHAR* s);
        void append(const TCHAR* s);
        int size() const;

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};

	class ListViewControl : public syscontrol
	{
    public:
        typedef std::function<LRESULT(int item, int subitem)> OnItemMouseEvent;

    public:
        struct Column
        {
            string id;
            string name;
            bool show;
            int width;
            int index;
            bool valid;

            Column(const TCHAR* name_, bool show_, int width_, const TCHAR* id_)
                : name(name_)
                , show(show_)
                , width(width_)
                , id(id_)
            {
                valid = true;
            }
        };
        
        class ColumnManager
        {
        public:
            struct ColumnFlags
            {
                enum Value
                {
                    All,
                    Available,
                    Showing,
                };
            };

            typedef std::vector<Column> ColumnContainer;
            typedef std::vector<int> TypeIndices;

        public:
            template<typename ...T>
            void push(T... args) {
                _columns.emplace_back(std::forward<T>(args)...);
                _columns.back().index = (int)_columns.size()-1;
            }
            Column& operator[](size_t i) { return _columns[i]; }
            size_t size() const { return _columns.size(); }

            void update();
            void each_all(std::function<void(int i, Column& c)> fn);
            void each_avail(std::function<void(int i, Column& c)> fn);
            void each_showing(std::function<void(int i, Column& c)> fn);
            void show(int available_index, int* listview_index);
            void hide(bool is_listview_index, int index, int* listview_delete_index);

            bool any_showing() const {return !_showing_indices.empty(); }
            Column& showing(int listview_index) { return _columns[_showing_indices[listview_index]]; }
            Column& avail(int index) { return _columns[_available_indices[index]]; }

            ColumnContainer& all() { return _columns; }

        protected:
            ColumnContainer _columns;
            TypeIndices     _available_indices;
            TypeIndices     _showing_indices;
        };

        class IDataSource
        {
        public:
            virtual size_t size() const = 0;
            virtual LPCTSTR get(int item, int subitem) const = 0;
        };

	public:
        bool redraw_items(int first, int last);
        bool set_item_count(int count, int flags);
        int get_item_count();
        int get_selected_count();
        int get_next_item(int start, unsigned int flags);
        unsigned int get_item_state(int i, unsigned int flag);
        string get_item_text(int i, int isub);
        void set_item_text(int i, int isub, const TCHAR* text);
        int get_item_data(int i, int isub);
		int insert_column(LPCTSTR name, int cx, int i);
        void delete_column(int i);
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
        ColumnManager& get_column_manager() { return _columns; }
        void update_columns();
        void set_column_width(int i, int cx);
        int get_column_width(int i);
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
		int get_column_order(int n, int* a);
        void set_column_order(int n, int* a);
        int subitem_hittest(LVHITTESTINFO* pht);
        void set_source(IDataSource* source) { _data = source; update_source(); }
        void update_source() { set_item_count(_data->size(), 0); }

        // return non-zero on success
        int get_subitem_rect(int item, int subitem, RECT* rc, int code = LVIR_BOUNDS);
        bool showtip_needed(const POINT& pt, const TCHAR** s);

        void go_top() { ::PostMessage(_hwnd, WM_KEYDOWN, VK_HOME, 0); }
        void go_bottom() { ::PostMessage(_hwnd, WM_KEYDOWN, VK_END, 0); }

        void on_double_click(OnItemMouseEvent callback) { _on_dblclick = callback; }

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
        virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) override;

	private:
        ColumnManager       _columns;
        IDataSource*        _data;
        OnItemMouseEvent    _on_dblclick;
	};

	class tabctrl : public syscontrol
	{
    public:
        // 插入一个标签
        // i: 索引，从 0 开始
        // s: 索引文本
        // p: 用户数据
        int insert_item(int i, const TCHAR* s, const void* p);

        // 获取窗口/显示区域大小
        // large: 是求大的还是小的（大的：根据显示区域大小写TAB控件窗口大小）
        // rc: 传入、传出
        void adjust_rect(bool large, RECT* rc);

	protected:
        virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
	};
}

#endif//__taowin_syscontrols_h__
