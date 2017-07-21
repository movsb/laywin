#pragma once

namespace taowin {

class Button : public SystemControl
{
public:
    typedef std::function<void()> OnMouseEvent;

public:
    void set_text(const TCHAR* text) {
        ::SetWindowText(_hwnd, text);
    }

    void on_click(OnMouseEvent callback) { _on_click = callback; }

protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
    virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) override;

protected:
    OnMouseEvent    _on_click;
};

class RadioButton : public SystemControl
{
public:
    RadioButton();

protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;

protected:
    bool _b_has_group;
};

class CheckBox : public SystemControl
{
public:
    bool get_chekc();

    virtual void set_attr(const TCHAR* name, const TCHAR* value) override;

protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
};

class Label : public SystemControl
{
public:
    void set_text(const TCHAR* s)
    {
        ::SetWindowText(_hwnd, s);
    }

protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
};

class Group : public SystemControl
{
protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
};

}
