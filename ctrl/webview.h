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
    long        _nRefs;                         // ���ü���
    Delegate*   _pDelegate;                     // �¼��йܴ�����
    IDispatch*  _pDispOfWB;                     // IDispatch of IWebBrowser2
    bool        _bCanGoBack, _bCanGoForward;    // �Ƿ���Ժ�����ǰ��

};

// ʵ�� OLE �����Ľӿ�
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

    // �������������
    bool Create(HWND hOwner);

    // ��ȡ������
    HWND GetOwner() const;

    void Destroy();

    // ��Ϣ����
    bool FilterMessage(MSG* pMsg);

    // ��ѯ���ڵ� IWebBrowser2* ָ��
    IWebBrowser2* GetWebBrowser() const;

    void SetPos(const RECT& r);

public:
    // ����
    void Navigate(const wchar_t* url);

    // ����
    void GoBack();

    // ǰ��
    void GoForward();

    // ��ҳ
    void GoHome();

    // ˢ��
    void Refresh(bool bForce = false);

    // ֹͣ
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
    long                        _nRefs;                         // ���ü���
    unsigned int                _state;                         // �ڲ�״̬
    HWND                        _hOwner;                        // ������
    HWND                        _hWndIE;                        // Internet Explorer_Server ���ڵľ��
                                                                // �Ҳ�֪����ȷ��ȡ�ķ����������ڴ˱���һ�Σ�����Ϣ����ʹ��

    IStorage*                   _pStorage;                      // ���ڴ洢����
    IOleObject*                 _pOleObject;                    // OLE����
    IWebBrowser2*               _pWebBrowser;                   // IWebBrowser2*
    IOleInPlaceObject*          _pOleInPlaceObject;             // ��λ����
    IOleInPlaceActiveObject*    _pOleInPlaceActiveObject;       // ��λ�������

    WebBrowserEventsHandler*    _pEventsHandler;

    DWORD                       _dwDWebBrowserEvents2Cookie;    // DIID_DWebBrowserEvents2 cookie

protected:
    bool                        _bEnableContextMenus;           // �Ƿ�������ʾ�Ҽ��˵�
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
