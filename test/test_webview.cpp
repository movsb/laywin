#include <taowin/core/tw_taowin.h>

namespace {

class TW : public taowin::window_creator
{
public:
    TW()
    {
    }

protected:
    virtual void get_metas(WindowMeta* metas) override
    {
        __super::get_metas(metas);
        metas->style |= WS_CLIPCHILDREN;
    }
    virtual LPCTSTR get_skin_xml() const override
    {
        LPCTSTR json = _T(R"tw(
<window title="taowin演示窗口" size="800,600">
    <res>
        <font name="default" face="微软雅黑" size="12"/>
        <font name="1" face="微软雅黑" size="12"/>
    </res>
    <root>
        <horizontal>
            <webview name="c" url="https://blog.twofei.com/" />
        </horizontal>
    </root>
</window>
)tw");
        return json;
    }

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override
    {
        switch(umsg) {
        case WM_CREATE:
        {
            _c = _root->find<taowin::webview>(_T("c"));

            _c->add_callable(L"test", [this](taowin::DispParamsVisitor args, VARIANTARG* result) {
                msgbox(_T("calling test"));
                _c->fire_event(L"event", 0, nullptr);
                return S_OK;
            });

            _c->on_before_navigate([this](const wchar_t* uri, bool top) {
                LogLog(_T("开始导航：[%d] %s"), top, uri);
            });
            _c->on_navigate_complete([this](const wchar_t* uri, bool top) {
                LogLog(_T("导航完成：[%d] %s"), top, uri);
            });
            _c->on_document_complete([this](const wchar_t* uri, bool top) {
                LogLog(_T("文档完成：[%d] %s"), top, uri);
                _c->exec_script(LR"(
                    alert('execScript');
                    external.AddListener('event', function() { alert('event received'); });
                    external.test('string', 1234,true);
                )");
            });
            _c->on_new_window([this](const wchar_t* uri, const wchar_t* ref, bool* cancel, IDispatch** disp) {
                LogLog(_T("新开窗口：uri: %s, ref: %s"), uri, ref);
            });
            _c->on_title_change([this](const wchar_t* title) {
                LogLog(_T("标题改变：%s"), title);
            });
            _c->on_set_status_text([this](const wchar_t* text) {
                LogLog(_T("状态改变：%s"), text);
            });

            ::SetTimer(_hwnd, 0, 1000, nullptr);
            return 0;
        }
        /*
        case WM_TIMER:
        {
            if(wparam == 0) {
                HWND hFocus = ::GetFocus();
                if(hFocus) {
                    TCHAR cls[MAX_PATH];
                    ::GetClassName(hFocus, cls, _countof(cls));
                    LogLog(_T("焦点窗口：%s"), cls);
                }
                return 0;
            }
            break;
        }*/
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual taowin::syscontrol* filter_control(HWND hwnd) override
    {
        return nullptr;
    }

    virtual bool filter_message(MSG* msg) override
    {
        return _c->filter_message(msg);

    }

private:
    taowin::webview* _c;
};

}

void test_webview()
{
    auto w = new TW;
    w->create();
    w->show();
}
