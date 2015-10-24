#include <iostream>
#include <process.h>
#include <string>
#include <memory>
#include <functional>
#include <fstream>

#include "lw_laywin.h"
#include <commctrl.h>

class TW : public laywin::window_creator
{
public:
	TW()
	{}

protected:
	virtual LPCTSTR get_skin_json() const
	{
        LPCTSTR json = R"(
<window title="taowin演示窗口" size="500,300">
    <res>
        <font name="default" face="微软雅黑" size="12"/>
    </res>
    <root>
        <horizontal>
            <button name="t1" text="t1" size="100,100"/>
            <button text="t2"/>
            <horizontal>
                <edit text="edit1" exstyle="clientedge" maxwidth="100"/>
                <edit text="说得好像真的一样，然而并没有什么卵用。" exstyle="clientedge"/>
            </horizontal>
        </horizontal>
    </root>
</window>
)";
		return json;
	}

	virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override
	{
		switch(umsg)
		{
        case WM_LBUTTONUP:
        {
            auto tw = new TW();
            int r = tw->domodal({"", "taowin", WS_OVERLAPPEDWINDOW, 0}, _hwnd);
            return 0;
        }
		case WM_CREATE:
		{
			center();

			return 0;
		}
        case WM_CLOSE:
            if(MessageBox(_hwnd, "确认关闭？", "", MB_OKCANCEL) != IDOK) {
                return 0;
            }
		default:
			break;
		}
        return __super::handle_message(umsg, wparam, lparam);
	}

    virtual LRESULT on_notify(HWND hwnd, laywin::control* pc, int code, NMHDR* hdr) override {
        if(pc->name() == "t1") {
            if(code == BN_CLICKED) {
                MessageBox(_hwnd, "t1 clicked", "", MB_OK);
                return 0;
            }
        }
        return 0;
    }
};

#ifdef _DEBUG
int main()
#else
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#endif
{
    laywin::init();

    printf("running...\n");

	try{
		TW tw1;
        tw1.create({ "test", "taowin", WS_OVERLAPPEDWINDOW, 0});
		tw1.show();


        laywin::loop_message();
	}
	catch(LPCTSTR e){
		std::wcout << e << std::endl;
	}
	
	return 0;
}
