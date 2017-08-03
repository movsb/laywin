#pragma once

namespace taowin {

class TextBox : public SystemControl
{
public:
    TextBox();

    // TODO RichEdit inherits from this, make it virtual
    ~TextBox();

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

    HBRUSH handle_ctlcoloredit(HDC hdc);

    void on_change(OnNotify callback) { _on_change = callback; }

protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
    virtual void set_attr(const TCHAR* name, const TCHAR* value) override;
    virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) override;

private:
    OnNotify    _on_change;

private:
    COLORREF    _crTextColor;
    COLORREF    _crBackgroundColor;
    HBRUSH      _hBrushBackground;
};

}
