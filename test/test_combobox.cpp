#include <windows.h>
#include <commctrl.h>

#include "../etwlogger.h"
#include "core/tw_taowin.h"


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
        <font name="1" face="Î¢ÈíÑÅºÚ" size="12"/>
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <combobox name="c" />
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
            _c = _root->find<taowin::ComboboxControl>(_T("c"));
            _c->add_string(_T("111"));
            _c->add_string(_T("222"));
            _c->add_string(_T("333"));

            _c->on_sel_change([this](int index, void* ud) {
                EtwLog(_T("Ñ¡ÖÐ¸Ä±ä %d,%p"), index, ud);
                return 0;
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

    taowin::ComboboxControl* _c;
};

void test_combobox()
{
    auto w = new TW;
    w->create();
    w->show();
}
