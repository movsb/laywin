#include <windows.h>
#include <commctrl.h>

#include <taowin/core/tw_taowin.h>

namespace {

class TW : public taowin::window_creator
{
public:
	TW()
	{}

protected:
	virtual LPCTSTR get_skin_xml() const override
	{
        LPCTSTR json = _T(R"tw(
<window title="taowinÑÝÊ¾´°¿Ú" size="500,300">
    <res>
        <font name="default" face="Î¢ÈíÑÅºÚ" size="12"/>
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <tabctrl name="c" />
        </vertical>
    </root>
</window>
)tw");
		return json;
	}

	virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override
	{
		switch(umsg)
		{
		case WM_CREATE:
		{
            _c = _root->find<taowin::tabctrl>(_T("c"));
            _c->insert_item(0, _T("Tab1"), nullptr);
            _c->insert_item(1, _T("Tab2"), nullptr);
            _c->insert_item(2, _T("Tab3"), nullptr);

			taowin::button* p = nullptr;
			p->set_text(_T("laskd"));
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
    taowin::tabctrl* _c;
};

}

void test_tabctrl()
{
    auto w = new TW;
    w->create();
    w->show();
}
