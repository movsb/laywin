#include <windows.h>
#include <commctrl.h>

#include "../etwlogger.h"
#include "core/tw_taowin.h"

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
            <progress height="40" name="c" range="0,100" color="255,0,0" bkcolor="255,255,0" />
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
            _c = _root->find<taowin::progress>(_T("c"));
            _c->set_pos(50);
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
    taowin::progress* _c;
};

}

void test_progress()
{
    auto w = new TW;
    w->create();
    w->show();
}
