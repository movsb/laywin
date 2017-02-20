#include "../etwlogger.h"
#include "core/tw_taowin.h"

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
<window title="taowinÑÝÊ¾´°¿Ú" size="500,300">
    <res>
        <font name="default" face="Î¢ÈíÑÅºÚ" size="12"/>
        <font name="1" face="Î¢ÈíÑÅºÚ" size="12"/>
    </res>
    <root>
        <webview name="c" url="https://blog.twofei.com/test" />
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

            return 0;
        }
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual taowin::syscontrol* filter_control(HWND hwnd) override
    {
        return nullptr;
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
