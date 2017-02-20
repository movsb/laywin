#include <windows.h>
#include <commctrl.h>

#include "../etwlogger.h"
#include "../src/tw_taowin.h"

namespace {

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
    {
    }

protected:
    virtual LPCTSTR get_skin_xml() const override
    {
        LPCTSTR json = R"tw(
<window title="taowin演示窗口" size="500,300">
    <res>
        <font name="default" face="微软雅黑" size="12"/>
        <font name="1" face="微软雅黑" size="12"/>
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
        switch(umsg) {
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

            lv->on_custom_draw([this](NMHDR* hdr) {
                EtwLog(_T("自绘"));
                return 0;
            });

            lv->on_double_click([this](int item, int subitem) {
                EtwLog(_T("双击（%d, %d）"), item, subitem);
                return 0;
            });

            lv->on_header_divider_dblclick([this](NMHDR* hdr) {
                EtwLog(_T("表头分隔线双击"));
                return 0;
            });

            lv->on_header_end_drag([this]() {
                EtwLog(_T("结束拖放"));
                return 0;
            });

            lv->on_header_end_track([this](NMHDR* hdr) {
                EtwLog(_T("结束拖动"));
                return 0;
            });

            lv->on_header_rclick([this]() {
                EtwLog(_T("表头右键单击"));
                return 0;
            });

            lv->on_item_changed([this](NMHDR* hdr) {
                EtwLog(_T("状态改变"));
                return 0;
            });

            lv->on_key_down([this](NMHDR* hdr) {
                EtwLog(_T("按键按下"));
                return 0;
            });

            lv->on_right_click([this](int item, int subitem) {
                EtwLog(_T("右键单击 %d,%d"), item, subitem);
                return 0;
            });

            return 0;
        }
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual taowin::syscontrol* filter_control(HWND hwnd) override
    {
        auto lv = _root->find<taowin::ListViewControl>("lv");
        if(hwnd == lv->get_header()) return lv;
        return nullptr;
    }
private:
    DataSource _data;
};

}

void test_listview()
{
    auto w = new TW;
    w->create();
    w->show();
}
