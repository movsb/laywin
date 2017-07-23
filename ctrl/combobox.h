#pragma once

namespace taowin {
class ComboBox : public SystemControl
{
public:
    typedef std::function<void(ComboBox* that, DRAWITEMSTRUCT* dis, int i, bool selected)> OnDraw;
    typedef std::function<LRESULT(int index, void* ud)> OnSelChange;

    struct IDataSource
    {
        // 取得数据数量
        virtual size_t      Size() = 0;

        // 取得位于 index 偏移（从 0 开始）的项数据
        virtual void        GetAt(size_t index, TCHAR const** text, void** tag) = 0;
    };

public:
    ComboBox();
    ~ComboBox();

    int add_string(const TCHAR* s);
    int add_string(const TCHAR* s, void* data);
    void set_item_data(int i, void* data);
    void* get_item_data(int i);
    void* get_cur_data();

    // 获取当前选中索引
    int get_cur_sel();

    // 设置当前选中索引
    void set_cur_sel(int i);    // 为指定索引号
    void set_cur_sel(void* p);  // 为指定数据


    int get_count();
    string get_text();
    void clear();
    void set_ondraw(OnDraw fn) { _ondraw = fn; }
    void drawit(DRAWITEMSTRUCT* dis);

    // 重新装载当前数据源
    void reload();

    // 设置数据源
    void set_source(IDataSource* source);

public:
    void on_sel_change(OnSelChange callback) { _on_sel_change = callback; }

private:
    void adjust_droplist_width();

protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
    virtual void set_attr(const TCHAR* name, const TCHAR* value) override;
    virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) override;

private:
    IDataSource*        _source;
    OnDraw              _ondraw;
    OnSelChange         _on_sel_change;
};
}