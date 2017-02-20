#pragma once

#include <cassert>

#include <string>
#include <tuple>

#include <windows.h>
#include <exdisp.h>
#include <ExDispid.h>
#include <mshtmhst.h>

#include "util/com_hlp.hpp"
#include "core/tw_window.h"

namespace taowin {

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

class WebBrowserEventsHandler
    : public DWebBrowserEvents2
{
    friend class WebBrowserContainer;

public:
    class Delegate
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

public:
    WebBrowserEventsHandler();
    virtual ~WebBrowserEventsHandler();

    void SetDelegate(Delegate* pDelegate) { _pDelegate = pDelegate; }
    void SetWebBrowser(IDispatch* pDisp) { _pDispOfWB = pDisp; }

public:
    // IUnknown methods
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    virtual STDMETHODIMP_(ULONG) AddRef() override;
    virtual STDMETHODIMP_(ULONG) Release() override;

    // IDispatch methods
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) override;
    virtual STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) override;
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) override;
    virtual STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override;

protected:
    long        _nRefs;                         // 引用计数
    Delegate*   _pDelegate;                     // 事件托管处理者
    IDispatch*  _pDispOfWB;                     // IDispatch of IWebBrowser2
    bool        _bCanGoBack, _bCanGoForward;    // 是否可以后退与前进

};

// 实现 OLE 容器的接口
// https://en.wikipedia.org/wiki/Object_Linking_and_Embedding#OLE_container
class WebBrowserContainer
    : public IOleClientSite
    , public IOleInPlaceSite
    , public IOleInPlaceFrame
    , public IDocHostUIHandler
{
    struct BrowserState
    {
        typedef const unsigned int Type;
        static Type fail = 0x00000001;
        static Type stopped = 0x00000002;
    };

public:
    WebBrowserContainer();
    virtual ~WebBrowserContainer();
    WebBrowserContainer(const WebBrowserContainer&) = delete;
    void operator=(const WebBrowserContainer&) = delete;

    void SetDelegate(WebBrowserEventsHandler::Delegate* pDelegate) { _pEventsHandler->SetDelegate(pDelegate); }

    unsigned int SetStatus(BrowserState::Type addend = 0, BrowserState::Type sub = 0);

    // 创建浏览器对象
    bool Create(HWND hOwner);

    // 获取父窗口
    HWND GetOwner() const;

    void Destroy();

    // 消息过滤
    bool FilterMessage(MSG* pMsg);

    // 查询窗口的 IWebBrowser2* 指针
    IWebBrowser2* GetWebBrowser() const;

    void SetPos(const RECT& r);

public:
    // 导航
    void Navigate(const wchar_t* url);

    // 后退
    void GoBack();

    // 前进
    void GoForward();

    // 主页
    void GoHome();

    // 刷新
    void Refresh(bool bForce = false);

    // 停止
    void Stop();

public:
    // IUnknown methods
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    virtual STDMETHODIMP_(ULONG) AddRef() override;
    virtual STDMETHODIMP_(ULONG) Release() override;

protected:
    // IOleClientSite methods
    virtual STDMETHODIMP SaveObject() override;
    virtual STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk) override;
    virtual STDMETHODIMP GetContainer(IOleContainer **ppContainer) override;
    virtual STDMETHODIMP ShowObject() override;
    virtual STDMETHODIMP OnShowWindow(BOOL fShow) override;
    virtual STDMETHODIMP RequestNewObjectLayout() override;

    // IOleInPlaceSite methods
    virtual STDMETHODIMP GetWindow(HWND *phwnd) override;
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) override;
    virtual STDMETHODIMP CanInPlaceActivate() override;
    virtual STDMETHODIMP OnInPlaceActivate() override;
    virtual STDMETHODIMP OnUIActivate() override;
    virtual STDMETHODIMP GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    virtual STDMETHODIMP Scroll(SIZE scrollExtant) override;
    virtual STDMETHODIMP OnUIDeactivate(BOOL fUndoable) override;
    virtual STDMETHODIMP OnInPlaceDeactivate() override;
    virtual STDMETHODIMP DiscardUndoState() override;
    virtual STDMETHODIMP DeactivateAndUndo() override;
    virtual STDMETHODIMP OnPosRectChange(LPCRECT lprcPosRect) override;

    // IOleInPlaceFrame methods
    virtual STDMETHODIMP GetBorder(LPRECT lprectBorder) override;
    virtual STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pborderwidths) override;
    virtual STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pborderwidths) override;
    virtual STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName) override;
    virtual STDMETHODIMP InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) override;
    virtual STDMETHODIMP SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject) override;
    virtual STDMETHODIMP RemoveMenus(HMENU hmenuShared) override;
    virtual STDMETHODIMP SetStatusText(LPCOLESTR pszStatusText) override;
    virtual STDMETHODIMP TranslateAccelerator(LPMSG lpmsg, WORD wID) override;

    // IDocHostUIHandler
    virtual STDMETHODIMP ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved) override;
    virtual STDMETHODIMP GetHostInfo(DOCHOSTUIINFO *pInfo) override;
    virtual STDMETHODIMP ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc) override;
    virtual STDMETHODIMP HideUI(void) override;
    virtual STDMETHODIMP UpdateUI(void) override;
    virtual STDMETHODIMP EnableModeless(BOOL fEnable) override;
    virtual STDMETHODIMP OnDocWindowActivate(BOOL fActivate) override;
    virtual STDMETHODIMP OnFrameWindowActivate(BOOL fActivate) override;
    virtual STDMETHODIMP ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow) override;
    virtual STDMETHODIMP TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID) override;
    virtual STDMETHODIMP GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw) override;
    virtual STDMETHODIMP GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget) override;
    virtual STDMETHODIMP GetExternal(IDispatch **ppDispatch) override;
    virtual STDMETHODIMP TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut) override;
    virtual STDMETHODIMP FilterDataObject(IDataObject *pDO, IDataObject **ppDORet) override;

private:
    long                        _nRefs;                         // 引用计数
    unsigned int                _state;                         // 内部状态
    HWND                        _hOwner;                        // 父窗口
    HWND                        _hWndIE;                        // Internet Explorer_Server 窗口的句柄
                                                                // 我不知道正确获取的方法，所以在此保存一次，供消息过滤使用

    IStorage*                   _pStorage;                      // 所在存储对象
    IOleObject*                 _pOleObject;                    // OLE对象
    IWebBrowser2*               _pWebBrowser;                   // IWebBrowser2*
    IOleInPlaceObject*          _pOleInPlaceObject;             // 在位对象
    IOleInPlaceActiveObject*    _pOleInPlaceActiveObject;       // 在位激活对象

    WebBrowserEventsHandler*    _pEventsHandler;

    DWORD                       _dwDWebBrowserEvents2Cookie;    // DIID_DWebBrowserEvents2 cookie

protected:
    bool                        _bEnableContextMenus;           // 是否允许显示右键菜单
};

class webview:public syscontrol
{
public:
    void navigate(const TCHAR* url)
    {
        _pwb->Navigate(url);
    }

protected:
    virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override
    {
        metas.classname = _T("taowin::control");
        metas.before_creation = [this] {
            static WebBrowserVersionSetter dummy;
        };
        metas.after_created = [this] {
            _pwb = new WebBrowserContainer;
            _pwb->Create(_hwnd);
        };
    }

    virtual void set_attr(const TCHAR* name, const TCHAR* value) override
    {
        if(is_attr(_T("url"))) {
            navigate(value);
        }
        else {
            return __super::set_attr(name, value);
        }
    }

    virtual void pos(const Rect& rc) override
    {
        __super::pos(rc);
        _pwb->SetPos(rc);
    }

protected:
    WebBrowserContainer* _pwb;
};

}
