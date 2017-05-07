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
<window title="taowin演示窗口" size="500,300">
    <res>
        <font name="default" face="微软雅黑" size="12"/>
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <edit name="c" />
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
            _c = _root->find<taowin::edit>(_T("c"));
            _c->on_change([this] {
                LogLog(_T("内容改变：%s\n"), _c->get_text().c_str());
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

private:
    taowin::edit* _c;
};

}

void test_edit()
{
    auto w = new TW;
    w->create();
    w->show();
}
