#include <windows.h>
#include <commctrl.h>

#include "../src/tw_taowin.h"

class TW : public taowin::window_creator
{
public:
	TW()
	{}

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
            <button name="t1" text="È¡Ïû" font="1"/>
            <listview name="lv" style="singlesel" exstyle="clientedge">  </listview>
            <horizontal>
            </horizontal>
        </vertical>
    </root>
</window>
)tw";
		return json;
	}

	virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override
	{
		switch(umsg)
		{
		case WM_CREATE:
		{
			//center();
            auto lv = _root->find<taowin::listview>("lv");
            lv->insert_column("c1", 50, 0);
            lv->insert_column("c2", 50, 1);
            lv->insert_column("c3", 50, 2);
            lv->insert_item("line1");
            lv->insert_item("line2");
            lv->insert_item("line3");
			return 0;
		}
        case WM_CLOSE:
            if(MessageBox(_hwnd, "È·ÈÏ¹Ø±Õ£¿", "", MB_OKCANCEL) != IDOK) {
                return 0;
            }
		default:
			break;
		}
        return __super::handle_message(umsg, wparam, lparam);
	}

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override {
        if(pc->name() == "t1") {
            if (code == BN_CLICKED) {
                async_call([&]() {
                    msgbox("async calling");
                    async_call([this]() {
                        msgbox("another async calling");
                    });
                });
                return 0;
            }
        }
        return 0;
    }
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    taowin::init();

    printf("running...\n");

	try{
		TW tw1;
        tw1.create();
		tw1.show();

        taowin::loop_message();
	}
	catch(LPCTSTR){

	}
	
	return 0;
}
