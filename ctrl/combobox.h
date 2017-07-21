#pragma once

namespace taowin {
class ComboBox : public SystemControl
{
public:
    typedef std::function<void(ComboBox* that, DRAWITEMSTRUCT* dis, int i, bool selected)> OnDraw;
    typedef std::function<LRESULT(int index, void* ud)> OnSelChange;

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

public:
    void on_sel_change(OnSelChange callback) { _on_sel_change = callback; }
    
protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
    virtual void set_attr(const TCHAR* name, const TCHAR* value) override;
    virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) override;

private:
    OnDraw      _ondraw;
    OnSelChange _on_sel_change;
};
}