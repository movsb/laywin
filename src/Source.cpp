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
<window title="taowin��ʾ����" size="500,300">
    <res>
        <font name="default" face="΢���ź�" size="12"/>
        <font name="1" face="΢���ź�" size="12"/>
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <button name="nothing" text="ȡ��" font="1"/>
            <button name="nothing" text="ȡ��" font="1"/>
            <listview name="lv" style="singlesel" exstyle="clientedge">  </listview>
            <horizontal>
                <button name="nothing" text="ȡ��" font="1" width="100" maxheight="300"/>
                <button name="t1" text="�ڱ߾�Ǻ���" padding="5,5,5,5" style="multiline" font="1" maxwidth="200"/>
                <button name="t1" text="�ڱ߾�Ǻ���" padding="5,5,5,5" font="1" maxwidth="200"/>
                <button name="nothing" text="ȡ��" font="1"/>
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
			center();
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
            if(MessageBox(_hwnd, "ȷ�Ϲرգ�", "", MB_OKCANCEL) != IDOK) {
                return 0;
            }
		default:
			break;
		}
        return __super::handle_message(umsg, wparam, lparam);
	}

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override {
        if(pc->name() == "t1") {
            if(code == BN_CLICKED) {
                TW* tw = new TW;
                tw->domodal(this);
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

		TW tw2;
        tw2.create();
		tw2.show();

        taowin::loop_message();
	}
	catch(LPCTSTR){

	}
	
	return 0;
}
