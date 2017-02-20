#include "core/tw_syscontrols.h"
#include "webview.h"

#pragma comment(lib, "version.lib")

#include "etwlogger.h"
#undef EtwLog
#define EtwLog(...) 

namespace taowin {
WebBrowserEventsHandler::WebBrowserEventsHandler()
    : _nRefs(1)
    , _pDelegate(nullptr)
    , _pDispOfWB(nullptr)
    , _bCanGoBack(false)
    , _bCanGoForward(false)
{

}

WebBrowserEventsHandler::~WebBrowserEventsHandler()
{

}

HRESULT WebBrowserEventsHandler::QueryInterface(REFIID riid, void ** ppvObject)
{
    *ppvObject = nullptr;

    if(riid == __uuidof(DWebBrowserEvents2))        *ppvObject = reinterpret_cast<DWebBrowserEvents2*>(this);

    if(*ppvObject) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG WebBrowserEventsHandler::AddRef()
{
    return ++_nRefs;
}

ULONG WebBrowserEventsHandler::Release()
{
    if(--_nRefs <= 0) {
        delete this;
        return 0;
    }

    return _nRefs;
}

// IDispatch methods
HRESULT WebBrowserEventsHandler::GetTypeInfoCount(UINT* pctinfo)
{
    return E_NOTIMPL;
}


STDMETHODIMP WebBrowserEventsHandler::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserEventsHandler::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserEventsHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO*, UINT*)
{
    HRESULT hr = E_NOTIMPL;

    if(!_pDelegate)
        return hr;

    switch(dispIdMember)
    {
    case DISPID_TITLECHANGE:
    {
        const wchar_t* t = pDispParams->rgvarg[0].bstrVal;
        t = t ? t : L"";

        _pDelegate->OnTitleChange(t);

        hr = S_OK;
        break;
    }

    case DISPID_DOCUMENTCOMPLETE:
    {
        bool top = pDispParams->rgvarg[1].pdispVal == _pDispOfWB;
        const wchar_t* uri = pDispParams->rgvarg[0].pvarVal->bstrVal;

        uri = uri ? uri : L"";

        _pDelegate->OnDocumentComplete(uri, top);

        hr = S_OK;

        break;
    }

    case DISPID_NAVIGATECOMPLETE2:
    {
        bool top = pDispParams->rgvarg[1].pdispVal == _pDispOfWB;
        const wchar_t* uri = pDispParams->rgvarg[0].pvarVal->bstrVal;

        uri = uri ? uri : L"";

        _pDelegate->OnNavigateComplete(uri, top);

        hr = S_OK;

        break;
    }

    case DISPID_BEFORENAVIGATE2:
    {
        bool top = pDispParams->rgvarg[6].pdispVal == _pDispOfWB;
        const wchar_t* uri = pDispParams->rgvarg[5].pvarVal->bstrVal;

        uri = uri ? uri : L"";

        _pDelegate->OnBeforeNavigate(uri, top);

        hr = S_OK;

        break;
    }

    case DISPID_NEWWINDOW3:
    {
        const wchar_t* new_uri = pDispParams->rgvarg[0].bstrVal;
        const wchar_t* ref_uri = pDispParams->rgvarg[1].bstrVal;
        VARIANT_BOOL* pCancel = pDispParams->rgvarg[3].pboolVal;
        IDispatch** ppDisp = pDispParams->rgvarg[4].ppdispVal;

        new_uri = new_uri ? new_uri : L"";
        ref_uri = ref_uri ? ref_uri : L"";

        bool canc = false;

        *ppDisp = nullptr;

        _pDelegate->OnNewWindow(new_uri, ref_uri, &canc, ppDisp);

        *pCancel = canc ? VARIANT_TRUE : VARIANT_FALSE;

        hr = S_OK;

        break;
    }

    case DISPID_COMMANDSTATECHANGE:
    {
        auto bEnable = pDispParams->rgvarg[0].boolVal != VARIANT_FALSE;
        auto command = (CommandStateChangeConstants)pDispParams->rgvarg[1].lVal;

        switch(command)
        {
        case CSC_NAVIGATEFORWARD:
            _bCanGoForward = bEnable;
            break;

        case CSC_NAVIGATEBACK:
            _bCanGoBack = bEnable;
            break;
        }

        _pDelegate->OnCommandStatusChange(_bCanGoBack, _bCanGoForward);

        hr = S_OK;

        break;
    }

    case DISPID_NAVIGATEERROR:
    {
        HRESULT code = pDispParams->rgvarg[1].pvarVal->intVal;

        const wchar_t* frame = pDispParams->rgvarg[2].pvarVal->bstrVal;
        if(!frame) frame = L"";

        const wchar_t* uri = pDispParams->rgvarg[3].pvarVal->bstrVal;
        if(!uri) uri = L"";

        bool top = pDispParams->rgvarg[4].pdispVal == _pDispOfWB;

        _pDelegate->OnLoadError(top, uri, code, L"");

        hr = S_OK;

        break;
    }

    case DISPID_WINDOWCLOSING:
    {
        // bool bIsChildWindow = pDispParams->rgvarg[1].boolVal != VARIANT_FALSE;
        VARIANT_BOOL* bCancel = pDispParams->rgvarg[0].pboolVal;

        *bCancel = VARIANT_TRUE;

        _pDelegate->OnWindowClosing();

        hr = S_OK;

        break;
    }


    } // switch

    return hr;
}

//////////////////////////////////////////////////////////////////////////

WebBrowserContainer::WebBrowserContainer()
    : _nRefs(1)
    , _hOwner(0)
    , _state(0)
    , _pStorage(nullptr)
    , _pOleObject(nullptr)
    , _pOleInPlaceObject(nullptr)
    , _pOleInPlaceActiveObject(nullptr)
    , _dwDWebBrowserEvents2Cookie(0)
    , _hWndIE(nullptr)
    , _bEnableContextMenus(true)
{
    _pEventsHandler = new WebBrowserEventsHandler;
}

WebBrowserContainer::~WebBrowserContainer()
{
    _pEventsHandler->Release();
    _pEventsHandler = nullptr;
}

unsigned int WebBrowserContainer::SetStatus(BrowserState::Type addend, BrowserState::Type sub)
{
    _state |= addend;
    _state &= ~sub;
    return _state;
}

bool WebBrowserContainer::Create(HWND hOwner)
{
    HRESULT hr = E_FAIL;

    _hOwner = hOwner;

    hr = StgCreateDocfile(nullptr, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_CREATE, 0, &_pStorage);
    if(FAILED(hr) || !_pStorage) {
        SetStatus(BrowserState::fail);
        return false;
    }

    hr = OleCreate(CLSID_WebBrowser, IID_IOleObject, OLERENDER_DRAW, 0, this, _pStorage, (void**)&_pOleObject);
    if(FAILED(hr) || !_pOleObject) {
        SetStatus(BrowserState::fail);
        return false;
    }

    RECT rc;
    ::GetClientRect(_hOwner, &rc);
    hr = _pOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, 0, this, 0, _hOwner, &rc);
    if(FAILED(hr)) {
        SetStatus(BrowserState::fail);
        return false;
    }

    hr = _pOleObject->QueryInterface(IID_PPV_ARGS(&_pWebBrowser));
    if(FAILED(hr) || !_pWebBrowser) {
        SetStatus(BrowserState::fail);
        return false;
    }

    hr = _pOleObject->QueryInterface(IID_PPV_ARGS(&_pOleInPlaceObject));
    if(FAILED(hr) || !_pOleInPlaceObject) {
        SetStatus(BrowserState::fail);
        return false;
    }

    hr = _pWebBrowser->QueryInterface(IID_PPV_ARGS(&_pOleInPlaceActiveObject));
    if(FAILED(hr) || !_pOleInPlaceActiveObject) {
        SetStatus(BrowserState::fail);
        return false;
    }

    //* 挂接DWebBrwoser2Event
    ComPtr<IConnectionPointContainer> pCPC;
    hr = _pWebBrowser->QueryInterface(IID_PPV_ARGS(&pCPC));
    if(SUCCEEDED(hr) && pCPC) {
        ComPtr<IConnectionPoint> pCP;
        if(SUCCEEDED(pCPC->FindConnectionPoint(DIID_DWebBrowserEvents2, &pCP)) && pCP) {
            hr = pCP->Advise((IUnknown*)(void*)this, &_dwDWebBrowserEvents2Cookie);
        }
    }

    if(FAILED(hr)) {
        SetStatus(BrowserState::fail);
        return false;
    }

    return SUCCEEDED(hr);
}

HWND WebBrowserContainer::GetOwner() const
{
    return _hOwner;
}

// http://stackoverflow.com/a/14652605/3628322
void WebBrowserContainer::Destroy()
{
    HRESULT hr = S_FALSE;

    Stop();

    if(_dwDWebBrowserEvents2Cookie) {
        ComPtr<IConnectionPointContainer> spCPC;
        hr = _pWebBrowser->QueryInterface(IID_IConnectionPointContainer, (void**)&spCPC);
        if(SUCCEEDED(hr) && spCPC) {
            ComPtr<IConnectionPoint> spCP;
            hr = spCPC->FindConnectionPoint(DIID_DWebBrowserEvents2, &spCP);
            if(SUCCEEDED(hr) && spCP) {
                spCP->Unadvise(_dwDWebBrowserEvents2Cookie);
                _dwDWebBrowserEvents2Cookie = 0;
            }
        }
    }

    if(_pWebBrowser) {
        _pWebBrowser->put_Visible(VARIANT_FALSE);
        _pWebBrowser->Release();
    }

    if(_pOleInPlaceActiveObject) {
        _pOleInPlaceActiveObject->Release();
        _pOleInPlaceActiveObject = nullptr;
    }

    if(_pOleInPlaceObject) {
        hr = _pOleInPlaceObject->InPlaceDeactivate();
        _pOleInPlaceObject->Release();
        _pOleInPlaceObject = nullptr;
    }

    if(_pOleObject) {
        _pOleObject->DoVerb(OLEIVERB_HIDE, nullptr, this, 0, _hOwner, nullptr);
        _pOleObject->Close(OLECLOSE_NOSAVE);
        OleSetContainedObject(_pOleObject, FALSE);
        _pOleObject->SetClientSite(nullptr);
        CoDisconnectObject(_pOleObject, 0);
        _pOleObject->Release();
    }

    if(_pStorage) {
        _pStorage->Release();
        _pStorage = nullptr;
    }
}

void WebBrowserContainer::Navigate(const wchar_t* url)
{
    if(_pWebBrowser) {
        ComVariant varUrl(url);
        _pWebBrowser->Navigate2(&varUrl, nullptr, nullptr, nullptr, nullptr);
    }
}

void WebBrowserContainer::GoBack()
{
    if(_pWebBrowser) {
        _pWebBrowser->GoBack();
    }
}

void WebBrowserContainer::GoForward()
{
    if(_pWebBrowser) {
        _pWebBrowser->GoForward();
    }
}

void WebBrowserContainer::GoHome()
{
    if(_pWebBrowser) {
        _pWebBrowser->GoHome();
    }
}

void WebBrowserContainer::Refresh(bool bForce)
{
    if(_pWebBrowser) {
        VARIANT type;
        type.vt = VT_I4;
        type.intVal = bForce ? REFRESH_COMPLETELY : REFRESH_NORMAL;
        _pWebBrowser->Refresh2(&type);
    }
}

void WebBrowserContainer::Stop()
{
    if(_pWebBrowser) {
        _pWebBrowser->Stop();
    }
}

bool WebBrowserContainer::FilterMessage(MSG* pMsg)
{
    bool bFiltered = false;

    bFiltered = _pOleInPlaceActiveObject && _pOleInPlaceActiveObject->TranslateAccelerator(pMsg) == S_OK;

    return bFiltered;
}

IWebBrowser2* WebBrowserContainer::GetWebBrowser() const
{
    return _pWebBrowser;
}

void WebBrowserContainer::SetPos(const RECT& r)
{
    if(_pOleObject && _pOleInPlaceObject) {
        SIZEL s = {r.right - r.left, r.bottom - r.top};
        _pOleObject->SetExtent(DVASPECT_CONTENT, &s);

        _pOleInPlaceObject->SetObjectRects(&r, &r);
    }
}

// IUnknown methods
STDMETHODIMP WebBrowserContainer::QueryInterface(REFIID riid, void** ppvObject)
{
    *ppvObject = nullptr;

    if(riid == IID_IUnknown)                        *ppvObject = this;
    else if(riid == IID_IOleClientSite)             *ppvObject = static_cast<IOleClientSite*>(this);
    else if(riid == IID_IOleInPlaceSite)            *ppvObject = static_cast<IOleInPlaceSite*>(this);
    else if(riid == IID_IOleInPlaceFrame)           *ppvObject = static_cast<IOleInPlaceFrame*>(this);
    else if(riid == IID_IOleInPlaceUIWindow)        *ppvObject = static_cast<IOleInPlaceUIWindow*>(this);
    else if(riid == IID_IDocHostUIHandler)          *ppvObject = static_cast<IDocHostUIHandler*>(this);

    if(*ppvObject) {
        AddRef();
        return S_OK;
    }

    HRESULT hr = E_NOINTERFACE;

    if(riid == DIID_DWebBrowserEvents2)             hr = _pEventsHandler->QueryInterface(riid, ppvObject);

    return hr;
}

STDMETHODIMP_(ULONG) WebBrowserContainer::AddRef()
{
    return ::InterlockedIncrement(&_nRefs);
}

STDMETHODIMP_(ULONG) WebBrowserContainer::Release()
{
    if(::InterlockedDecrement(&_nRefs) <= 0) {
        delete this;
        return 0;
    }

    return _nRefs;
}

// IOleClientSite methods
STDMETHODIMP WebBrowserContainer::SaveObject()
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::GetContainer(IOleContainer **ppContainer)
{
    EtwLog(L"Enter");
    return E_FAIL;
}

STDMETHODIMP WebBrowserContainer::ShowObject()
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::OnShowWindow(BOOL fShow)
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::RequestNewObjectLayout()
{
    EtwLog(L"Enter");
    return S_OK;
}

// IOleInPlaceSite methods
STDMETHODIMP WebBrowserContainer::GetWindow(HWND *phwnd)
{
    EtwLog(L"Enter");
    *phwnd = _hOwner;
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::ContextSensitiveHelp(BOOL fEnterMode)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::CanInPlaceActivate()
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::OnInPlaceActivate()
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::OnUIActivate()
{
    EtwLog(L"Enter");
    if(!_hWndIE)
    {
        // 窗口层次关系：BrowserWindow / Shell Embedding / Shell DocObject View / Internet Explorer_Server / <etc>
        _hWndIE = ::GetWindow(::GetWindow(::GetWindow(_hOwner, GW_CHILD), GW_CHILD), GW_CHILD);
        assert(_hWndIE != nullptr);
    }

    if(_pEventsHandler->_pDelegate) {
        _pEventsHandler->_pDelegate->OnFocusChange(true);
    }

    return S_OK;
}

STDMETHODIMP WebBrowserContainer::GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    EtwLog(L"Enter");
    *ppFrame = static_cast<IOleInPlaceFrame*>(this);
    AddRef();

    *ppDoc = nullptr;

    RECT rcClient;
    ::GetClientRect(_hOwner, &rcClient);
    *lprcPosRect = *lprcClipRect = rcClient;

    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = ::GetParent(_hOwner);
    lpFrameInfo->haccel = 0;
    lpFrameInfo->cAccelEntries = 0;

    return S_OK;
}

STDMETHODIMP WebBrowserContainer::Scroll(SIZE scrollExtant)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::OnUIDeactivate(BOOL fUndoable)
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::OnInPlaceDeactivate()
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::DiscardUndoState()
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::DeactivateAndUndo()
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::OnPosRectChange(LPCRECT lprcPosRect)
{
    EtwLog(L"Enter");
    return S_OK;
}

// IOleInPlaceFrame methods
STDMETHODIMP WebBrowserContainer::GetBorder(LPRECT lprectBorder)
{
    EtwLog(L"Enter");
    ::GetClientRect(_hOwner, lprectBorder);
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
    EtwLog(L"Enter");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::RemoveMenus(HMENU hmenuShared)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::SetStatusText(LPCOLESTR pszStatusText)
{
    EtwLog(L"%s", pszStatusText);
    // _pCallback->OnSetStatusText(pszStatusText ? pszStatusText : L"");
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::EnableModeless(BOOL fEnable)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

// IDocHostUIHandler
STDMETHODIMP WebBrowserContainer::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
    EtwLog(L"Enter");
    return _bEnableContextMenus
        ? S_FALSE   // 默认
        : S_OK      // 不允许
        ;
}

STDMETHODIMP WebBrowserContainer::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    EtwLog(L"Enter");
    pInfo->dwFlags |= DOCHOSTUIFLAG_NO3DOUTERBORDER;
    return S_OK;
}

STDMETHODIMP WebBrowserContainer::ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::HideUI(void)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::UpdateUI(void)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::OnDocWindowActivate(BOOL fActivate)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}
STDMETHODIMP WebBrowserContainer::OnFrameWindowActivate(BOOL fActivate)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::GetExternal(IDispatch **ppDispatch)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

STDMETHODIMP WebBrowserContainer::FilterDataObject(IDataObject *pDO, IDataObject **ppDORet)
{
    EtwLog(L"Enter");
    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////

WebBrowserVersionSetter::WebBrowserVersionSetter()
{
    InitInternetExplorer();
}

WebBrowserVersionSetter::~WebBrowserVersionSetter()
{
    UnInitInternetExplorer();
}

int WebBrowserVersionSetter::GetMSHTMLVersion()
{
    int v[4] = {0};
    const wchar_t* module = L"mshtml.dll";
    DWORD size = GetFileVersionInfoSizeW(module, nullptr);
    if(size) {
        std::unique_ptr<unsigned char[]> info(new unsigned char[size]);
        if(GetFileVersionInfoW(module, 0, size, info.get())) {
            VS_FIXEDFILEINFO* pFixedInfo;
            if(VerQueryValueW(info.get(), L"\\", (void**)&pFixedInfo, (UINT*)&size)) {
                v[0] = HIWORD(pFixedInfo->dwFileVersionMS);
                v[1] = LOWORD(pFixedInfo->dwFileVersionMS);
                v[2] = HIWORD(pFixedInfo->dwFileVersionLS);
                v[3] = LOWORD(pFixedInfo->dwFileVersionLS);
            }
        }
    }

    return v[0];
}

void WebBrowserVersionSetter::InitInternetExplorer()
{
    wchar_t path[MAX_PATH];
    ::GetModuleFileNameW(nullptr, path, _countof(path));
    _name = ::wcsrchr(path, L'\\') + 1;

    HKEY hKey;
    const wchar_t* key = LR"(Software\Microsoft\Internet Explorer\Main\FeatureControl)";
    if(::RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        HKEY hKeySub;
        if(::RegOpenKeyExW(hKey, L"FEATURE_BROWSER_EMULATION", 0, KEY_SET_VALUE, &hKeySub) == ERROR_SUCCESS) {
            DWORD dwVer = GetMSHTMLVersion() * 1000;
            ::RegSetValueExW(hKeySub, _name.c_str(), 0, REG_DWORD, (BYTE*)&dwVer, sizeof(DWORD));
            ::RegCloseKey(hKeySub);
        }
        if(::RegOpenKeyExW(hKey, L"FEATURE_TABBED_BROWSING", 0, KEY_SET_VALUE, &hKeySub) == ERROR_SUCCESS) {
            DWORD dwVal = 1;
            ::RegSetValueExW(hKeySub, _name.c_str(), 0, REG_DWORD, (BYTE*)&dwVal, sizeof(DWORD));
            ::RegCloseKey(hKeySub);
        }
        ::RegCloseKey(hKey);
    }
}

void WebBrowserVersionSetter::UnInitInternetExplorer()
{
    HKEY hKey;
    const wchar_t* key = LR"(Software\Microsoft\Internet Explorer\Main\FeatureControl)";
    if(::RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        HKEY hKeySub;
        if(::RegOpenKeyExW(hKey, L"FEATURE_BROWSER_EMULATION", 0, KEY_SET_VALUE, &hKeySub) == ERROR_SUCCESS) {
            ::RegDeleteValueW(hKeySub, _name.c_str());
            ::RegCloseKey(hKeySub);
        }
        if(::RegOpenKeyExW(hKey, L"FEATURE_TABBED_BROWSING", 0, KEY_SET_VALUE, &hKeySub) == ERROR_SUCCESS) {
            ::RegDeleteValueW(hKeySub, _name.c_str());
            ::RegCloseKey(hKeySub);
        }
        ::RegCloseKey(hKey);
    }
}

}
