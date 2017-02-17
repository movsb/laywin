#include <windows.h>
#include <commctrl.h>

#include "../etwlogger.h"
#include "../src/tw_taowin.h"

// {9CEC8287-B104-4A62-AF00-3DA900848A18}
static const GUID providerGUID = 
{ 0x9cec8287, 0xb104, 0x4a62, { 0xaf, 0x0, 0x3d, 0xa9, 0x0, 0x84, 0x8a, 0x18 } };
ETWLogger g_etwLogger(providerGUID);

class DataSource : public taowin::ListViewControl::IDataSource
{
protected:
    virtual size_t size() const override
    {
        return 100;
    }

    virtual LPCTSTR get(int item, int subitem) const override
    {
        static taowin::string text;
        text = !subitem ? std::to_string(item) : std::to_string(subitem);
        return text.c_str();
    }
};

class TW : public taowin::window_creator
{
public:
	TW()
	{}

protected:
	virtual LPCTSTR get_skin_xml() const override
	{
        LPCTSTR json = R"tw(
<window title="taowin—› æ¥∞ø⁄" size="500,300">
    <res>
        <font name="default" face="Œ¢»Ì—≈∫⁄" size="12"/>
        <font name="1" face="Œ¢»Ì—≈∫⁄" size="12"/>
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <listview name="lv" style="singlesel,ownerdata" exstyle="clientedge">  </listview>
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
            auto lv = _root->find<taowin::ListViewControl>("lv");
            auto& cm = lv->get_column_manager();
            cm.push("item1", true, 50, "id1");
            cm.push("item2", true, 100, "id2");
            cm.push("item3", true, 150, "id3");
            lv->update_columns();
            lv->set_source(&_data);
			return 0;
		}
        case WM_CLOSE:
            if(MessageBox(_hwnd, "»∑»œπÿ±’£ø", "", MB_OKCANCEL) != IDOK) {
                return 0;
            }
		default:
			break;
		}
        return __super::handle_message(umsg, wparam, lparam);
	}

private:
    DataSource _data;
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
