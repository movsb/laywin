#pragma once

namespace taowin {

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

    struct IDataSource
    {
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
    HeaderControl& get_header_control() { return _header; }
    int size() const;
    bool get_selected_items(std::vector<int>* items);
    void set_item_state(int i, int mask, int state);
    void ensure_visible(int i);
    void show_header(int state); // 1:show, 0, hide, -1: toggle
    bool is_header_visible();
    int get_top_index();
    bool get_item_position(int i, POINT* ppt);
    void scroll(int dx, int dy);
    int find_item(int start, const LVFINDINFO* lvfi);
    int get_column_order(int n, int* a);
    void set_column_order(int n, int* a);
    int subitem_hittest(LVHITTESTINFO* pht);
    void set_source(IDataSource* source) { _data = source; update_source(); }
    void update_source(int flags = 0) { set_item_count(_data->size(), flags); }

    // return non-zero on success
    int get_subitem_rect(int item, int subitem, RECT* rc, int code = LVIR_BOUNDS);
    bool showtip_needed(const POINT& pt, const TCHAR** s);

    void go_top() { ::PostMessage(_hwnd, WM_KEYDOWN, VK_HOME, 0); }
    void go_bottom() { ::PostMessage(_hwnd, WM_KEYDOWN, VK_END, 0); }

    void on_right_click(OnItemMouseEvent callback) { _on_right_click = callback; }
    void on_double_click(OnItemMouseEvent callback) { _on_dblclick = callback; }
    void on_custom_draw(OnHdrNotify callback) { _on_custom_draw = callback; }
    void on_key_down(OnHdrNotify callback) { _on_key_down = callback; }
    void on_item_changed(OnHdrNotify callback) { _on_item_changd = callback; }

    void on_header_rclick(OnNotify callback) { _header.on_rclick = callback; }
    void on_header_end_track(OnHdrNotify callback) { _header.on_end_track = callback; }
    void on_header_divider_dblclick(OnHdrNotify callback) { _header.on_divider_dblclick = callback; }
    void on_header_end_drag(OnNotify callback) { _header.on_end_drag = callback; }

protected:
    virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
    virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) override;
    virtual bool filter_child(HWND child, int code, NMHDR* hdr, LRESULT* lr) override;

private:
    HeaderControl       _header;
    ColumnManager       _columns;
    IDataSource*        _data;

    OnItemMouseEvent    _on_dblclick;
    OnHdrNotify         _on_custom_draw;
    OnItemMouseEvent    _on_right_click;
    OnHdrNotify         _on_key_down;
    OnHdrNotify         _on_item_changd;
};


}
