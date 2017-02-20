#pragma once

namespace taowin {

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

}
