#include <taowin/core/tw_taowin.h>

namespace {

class TW : public taowin::WindowCreator
{
public:
    TW()
    {
    }

protected:
    virtual LPCTSTR get_skin_xml() const override
    {
        LPCTSTR json = _T(R"tw(
<Window title="taowin��ʾ����" size="500,300">
    <Resource>
        <Font name="default" face="΢���ź�" size="12"/>
        <Font name="1" face="΢���ź�" size="12"/>
    </Resource>
    <Root>
        <Vertical padding="5,5,5,5">
            <Button name="btn1" text="��ť" width="100" height="50"/>
            <Button name="btn2" text="��ť" width="100" height="50"/>
            <Button name="btn3" text="��ť" width="100" height="50"/>
        </Vertical>
    </Root>
</Window>
)tw");
        return json;
    }

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override
    {
        switch(umsg) {
        case WM_CREATE:
		{
			taowin::Button *btn1, *btn2, *btn3;
            btn1 = _root->find<taowin::Button>(_T("btn1"));
            btn2 = _root->find<taowin::Button>(_T("btn2"));
            btn3 = _root->find<taowin::Button>(_T("btn3"));
            btn1->on_click([=] { LogLog(_T("��ť ��%s�� ���"), btn1->name().c_str()); });
            btn2->on_click([=] { LogLog(_T("��ť ��%s�� ���"), btn2->name().c_str()); });
            btn3->on_click([=] { LogLog(_T("��ť ��%s�� ���"), btn3->name().c_str()); });

            return 0;
        }
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual taowin::SystemControl* filter_control(HWND hwnd) override
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
