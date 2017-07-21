#include <windows.h>
#include <commctrl.h>

#include <taowin/core/tw_taowin.h>

namespace {

class TW : public taowin::WindowCreator
{
public:
	TW()
	{}

protected:
	virtual LPCTSTR get_skin_xml() const override
	{
        LPCTSTR json = _T(R"tw(
<Window title="taowinÑÝÊ¾´°¿Ú" size="500,300">
    <Resource>
        <Font name="default" face="Î¢ÈíÑÅºÚ" size="12"/>
    </Resource>
    <Root>
        <Vertical padding="5,5,5,5">
            <TabCtrl name="c" />
        </Vertical>
    </Root>
</Window>
)tw");
		return json;
	}

	virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override
	{
		switch(umsg)
		{
		case WM_CREATE:
		{
            _c = _root->find<taowin::TabCtrl>(_T("c"));
            _c->insert_item(0, _T("Tab1"), nullptr);
            _c->insert_item(1, _T("Tab2"), nullptr);
            _c->insert_item(2, _T("Tab3"), nullptr);

			taowin::Button* p = nullptr;
			p->set_text(_T("laskd"));
			return 0;
		}
		}
        return __super::handle_message(umsg, wparam, lparam);
	}

    virtual taowin::SystemControl* filter_control(HWND hwnd) override
    {
        return nullptr;
    }

private:
    taowin::TabCtrl* _c;
};

}

void test_tabctrl()
{
    auto w = new TW;
    w->create();
    w->show();
}
