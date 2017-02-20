#include <windows.h>
#include <commctrl.h>

#include "../etwlogger.h"
#include "../src/tw_taowin.h"

namespace {

class TW : public taowin::window_creator
{
public:
    TW()
    {
    }

protected:
    virtual LPCTSTR get_skin_xml() const override
    {
        LPCTSTR json = R"tw(
<window title="taowinÑÝÊ¾´°¿Ú" size="500,300">
    <res>
        <font name="default" face="Î¢ÈíÑÅºÚ" size="12"/>
        <font name="1" face="Î¢ÈíÑÅºÚ" size="12"/>
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <button name="c" />
        </vertical>
    </root>
</window>
)tw";
        return json;
    }

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override
    {
        switch(umsg) {
        case WM_CREATE:
        {
            _c = _root->find<taowin::button>("c");
            _c->on_click([this] {
                EtwLog(_T("°´Å¥µã»÷"));
            });

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
    taowin::button* _c;
};

}

void test_button()
{
    auto w = new TW;
    w->create();
    w->show();
}
