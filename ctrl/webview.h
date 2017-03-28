#pragma once

#include <taowin/util/com_hlp.hpp>

namespace taowin {

namespace _webview {

using Callable = std::function<ComRet(DispParamsVisitor args, VARIANT* result)>;

class WebBrowserVersionSetter
{
public:
    WebBrowserVersionSetter();
    ~WebBrowserVersionSetter();

protected:
    int GetMSHTMLVersion();
    void InitInternetExplorer();
    void UnInitInternetExplorer();

protected:
    std::wstring _name;
};

class EventDelegate
{
public:
    virtual void OnFocusChange(bool focus) {}
    virtual void OnBeforeNavigate(const wchar_t* uri, bool top) {}
    virtual void OnNewWindow(const wchar_t* uri, const wchar_t* ref, bool* pCancel, IDispatch** ppDisp) { *pCancel = false; *ppDisp = nullptr; }
    virtual void OnNavigateComplete(const wchar_t* uri, bool top) {}
    virtual void OnDocumentComplete(const wchar_t* uri, bool top) {}
    virtual void OnTitleChange(const wchar_t* title) {}
    virtual void OnFaviconURLChange(const wchar_t* uri) {}
    virtual void OnSetStatusText(const wchar_t* text) {}
    virtual void OnCommandStatusChange(bool cangoback, bool cangofwd) {}
    virtual void OnLoadError(bool is_main_frame, const wchar_t* url, int error_code, const wchar_t* error_text) {}
    virtual void OnWindowClosing() {}
};

class IWebBrowserContainer
{
public:
    virtual void Create(HWND hParent) = 0;
    virtual void Destroy() = 0;
    virtual void SetDelegate(EventDelegate* pDelegate) = 0;
    virtual void SetPos(const Rect& pos) = 0;
    virtual bool Focus() = 0;
    virtual bool FilterMessage(MSG* msg) = 0;

    virtual void Navigate(const wchar_t* url) = 0;
    virtual void GoHome() = 0;
    virtual void GoForward() = 0;
    virtual void GoBack() = 0;
    virtual void Refresh(bool force) = 0;
    virtual void Stop() = 0;

    virtual ComRet GetDocument(IHTMLDocument2** ppDocument) = 0;
	virtual std::wstring GetSource() = 0;
    virtual ComRet GetRootElement(IHTMLElement** ppElement) = 0;
    virtual ComRet ExecScript(const std::wstring& script, VARIANT* result, const std::wstring& lang) = 0;

    virtual void AddCallable(const wchar_t* name, Callable call) = 0;
    virtual void RemoveCallable(const wchar_t* name) = 0;
    virtual void FireEvent(const wchar_t* name, UINT argc, VARIANT* argv) = 0;

protected:

};


} // namespace _webview

class webview
    : public custom_control
    , private _webview::EventDelegate
{
public:
    void navigate(const TCHAR* url)
    {
        _pwb->Navigate(url);
    }

    ComRet exec_script(const TCHAR* script, VARIANT* result = nullptr)
    {
        return _pwb->ExecScript(script, result, _T(""));
    }

    std::wstring get_source()
    {
        return _pwb->GetSource();
    }

    bool filter_message(MSG* msg)
    {
        return _pwb->Focus() && _pwb->FilterMessage(msg);
    }

    void add_callable(const TCHAR* name, _webview::Callable call)
    {
        return _pwb->AddCallable(name, call);
    }

    void remove_callable(const TCHAR* name)
    {
        return _pwb->RemoveCallable(name);
    }

    void fire_event(const TCHAR* name, UINT argc = 0, VARIANT* argv = nullptr)
    {
        return _pwb->FireEvent(name, argc, argv);
    }

private:
    using OnBeforeNavigateT         = std::function<void(const wchar_t* uri, bool top)>;
    using OnNewWindwT               = std::function<void(const wchar_t* uri, const wchar_t* ref, bool* cancel, IDispatch** disp)>;
    using OnNavigateCompleteT       = std::function<void(const wchar_t* uri, bool top)>;
    using OnDocumentCompleteT       = std::function<void(const wchar_t* uri, bool top)>;
    using OnTitleChangeT            = std::function<void(const wchar_t* title)>;
    using OnSetStatusTextT          = std::function<void(const wchar_t* text)>;

    OnBeforeNavigateT               _on_before_navigate;
    OnNewWindwT                     _on_new_window;
    OnNavigateCompleteT             _on_navigate_complete;
    OnDocumentCompleteT             _on_document_complete;
    OnTitleChangeT                  _on_title_change;
    OnSetStatusTextT                _on_set_status_text;

    virtual void OnBeforeNavigate   (const wchar_t* uri, bool top){ if(_on_before_navigate) _on_before_navigate(uri, top); }
    virtual void OnNewWindow        (const wchar_t* uri, const wchar_t* ref, bool* pCancel, IDispatch** ppDisp) { if(_on_new_window) _on_new_window(uri, ref, pCancel, ppDisp); }
    virtual void OnNavigateComplete (const wchar_t* uri, bool top) { if(_on_navigate_complete) _on_navigate_complete(uri, top); }
    virtual void OnDocumentComplete (const wchar_t* uri, bool top) { if(_on_document_complete) _on_document_complete(uri, top); }
    virtual void OnTitleChange      (const wchar_t* title) { if(_on_title_change) _on_title_change(title); }
    virtual void OnSetStatusText    (const wchar_t* text) { if(_on_set_status_text) _on_set_status_text(text); }

public:
    void on_before_navigate         (OnBeforeNavigateT callback)        { _on_before_navigate       = callback; }
    void on_new_window              (OnNewWindwT callback)              { _on_new_window            = callback; }
    void on_navigate_complete       (OnNavigateCompleteT callback)      { _on_navigate_complete     = callback; }
    void on_document_complete       (OnDocumentCompleteT callback)      { _on_document_complete     = callback; }
    void on_title_change            (OnTitleChangeT callback)           { _on_title_change          = callback; }
    void on_set_status_text         (OnSetStatusTextT callback)         { _on_set_status_text       = callback; }

protected:
    virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
    virtual bool control_procedure(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lr) override;
    virtual void set_attr(const TCHAR* name, const TCHAR* value) override;
    virtual void pos(const Rect& rc) override;

protected:
    _webview::IWebBrowserContainer* _pwb;
};

}
