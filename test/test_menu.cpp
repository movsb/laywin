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
<Window title="taowin演示窗口" size="500,300">
    <Resource>
        <Font name="default" face="微软雅黑" size="12"/>
        <Font name="1" face="微软雅黑" size="12"/>
    </Resource>
    <Root>
        <Vertical padding="5,5,5,5">
            <Button name="btn1" text="弹出菜单" width="120" height="50"/>
            <Button name="btn2" text="切换第一项的选中状态" width="120" height="50"/>
            <Button name="btn3" text="切换第二项的启用状态" width="120" height="50"/>
            <Button name="btn4" text="清空弹出菜单" width="120" height="50"/>
            <Button name="btn5" text="删除item4" width="120" height="50"/>
            <Button name="btn6" text="插入分隔符到最后" width="120" height="50"/>
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
			taowin::Button *btn1;
            btn1 = _root->find<taowin::Button>(_T("btn1"));
            btn1->on_click([=] {
                LogLog(_T("按钮 「%s」 点击"), btn1->name().c_str());
                _menu->track();
            });

            taowin::Button* btn2 = _root->find<taowin::Button>(_T("btn2"));
            btn2->on_click([=] {
                auto item = _menu->find_sib(_T("item1"));
                item->check(-1);
                msgbox(item->is_checked() ? _T("checked") : _T("unchecked"));
            });

            taowin::Button* btn3 = _root->find<taowin::Button>(_T("btn3"));
            btn3->on_click([=] {
                auto item = _menu->find_sib(_T("item2"));
                item->enable(-1);
                msgbox(item->is_enabled() ? _T("enabled") : _T("disabled"));
            });

            taowin::Button* btn4 = _root->find<taowin::Button>(_T("btn4"));
            btn4->on_click([=] {
                auto item = _menu->find_sib(_T("item3"));
                item->clear();
            });

            taowin::Button* btn5 = _root->find<taowin::Button>(_T("btn5"));
            btn5->on_click([=] {
                auto item = _menu->find_sib(_T("item4"));
                item->remove();
            });

            taowin::Button* btn6 = _root->find<taowin::Button>(_T("btn6"));
            btn6->on_click([=] {
                _menu->insert_sep(_T("item3"), _T(""));
                _menu->insert_str(_T("item3"), _T(""), _T("insert menu text"), false, _T(""), true);
            });

            auto menustr = LR"==(
<MenuTree>
    <item id="item1" text="menu1" key="Ctrl+A"/>
    <item id="item2" text="menu1" key="Ctrl+A"/>
    <popup id="item3" text="submenu1">
        <item text="menu1" key="Ctrl+A"/>
        <item text="menu1" key="Ctrl+A"/>
        <item text="menu1" key="Ctrl+A"/>
    </popup>
    <item id="item4" text="menu4" />
</MenuTree>
)==";
            _menu = taowin::MenuItem::create(menustr);

            return 0;
        }
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual taowin::SystemControl* filter_control(HWND hwnd) override
    {
        return nullptr;
    }

    taowin::MenuItem* _menu;
};

}

void test_menu()
{
    auto w = new TW;
    w->create();
    w->show();
}
