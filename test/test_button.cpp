#include <taowin/core/tw_taowin.h>

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
        LPCTSTR json = _T(R"tw(
<window title="taowinÑÝÊ¾´°¿Ú" size="500,300">
    <res>
        <font name="default" face="Î¢ÈíÑÅºÚ" size="12"/>
        <font name="1" face="Î¢ÈíÑÅºÚ" size="12"/>
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <button name="btn1" text="°´Å¥" width="100" height="50"/>
            <button name="btn2" text="°´Å¥" width="100" height="50"/>
            <button name="btn3" text="°´Å¥" width="100" height="50"/>
        </vertical>
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
			taowin::button *btn1, *btn2, *btn3;
            btn1 = _root->find<taowin::button>(_T("btn1"));
            btn2 = _root->find<taowin::button>(_T("btn2"));
            btn3 = _root->find<taowin::button>(_T("btn3"));
            btn1->on_click([=] { LogLog(_T("°´Å¥ ¡¸%s¡¹ µã»÷"), btn1->name().c_str()); });
            btn2->on_click([=] { LogLog(_T("°´Å¥ ¡¸%s¡¹ µã»÷"), btn2->name().c_str()); });
            btn3->on_click([=] { LogLog(_T("°´Å¥ ¡¸%s¡¹ µã»÷"), btn3->name().c_str()); });

            return 0;
        }
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual taowin::syscontrol* filter_control(HWND hwnd) override
    {
        return nullptr;
    }
};

}

void test_button()
{
    auto w = new TW;
    w->create();
    w->show();
}
