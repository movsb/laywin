#include <windows.h>
#include <commctrl.h>

#include <taowin/core/tw_taowin.h>


class TW : public taowin::WindowCreator
{
public:
	TW()
	{}

    class DataSource : public taowin::ComboBox::IDataSource
    {
    public:
        virtual size_t Size() override
        {
            return _strs.size();
        }


        virtual void GetAt(size_t index, TCHAR const** text, void** tag) override
        {
            *text = _strs[index];
            *tag =  reinterpret_cast<void*>(std::stoi(*text));
        }

        DataSource()
        {
            _strs.emplace_back(_T("0"));
            _strs.emplace_back(_T("1"));
            _strs.emplace_back(_T("2"));
            _strs.emplace_back(_T("3"));
            _strs.emplace_back(_T("4"));
            _strs.emplace_back(_T("5"));
            _strs.emplace_back(_T("6"));
            _strs.emplace_back(_T("7"));
        }

        void reverse()
        {
            std::reverse(_strs.begin(), _strs.end());
        }

    protected:
        std::vector<const TCHAR*> _strs;
    };

protected:
	virtual LPCTSTR get_skin_xml() const override
	{
        LPCTSTR json = _T(R"tw(
<Window title="taowin演示窗口" size="500,300">
    <Resource>
        <Font name="default" face="微软雅黑" size="12"/>
        <Font name="1" face="微软雅黑" size="12"/>
    </Resource>
    <Root>
        <Vertical padding="5,5,5,5">
            <ComboBox name="c" />
            <Button name="btn" text="Reverse"/>
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
            _c = _root->find<taowin::ComboBox>(_T("c"));
            _c->set_source(&_source);
            _c->set_cur_sel(0);


            _c->on_sel_change([this](int index, void* ud) {
                LogLog(_T("选中索引改变 index=%d, tag=%p"), index, ud);
                return 0;
            });

            _root->find<taowin::Button>(_T("btn"))->on_click([this] {
                auto p = _c->get_cur_data();
                LogLog(_T("当前选中项：tag=%p"), p);
                _source.reverse();
                _c->reload();
                _c->set_cur_sel(p);
                LogLog(_T("Reverse"));
            });

			return 0;
		}
		}
        return __super::handle_message(umsg, wparam, lparam);
	}

    virtual taowin::SystemControl* filter_control(HWND hwnd) override
    {
        return nullptr;
    }

    DataSource _source;
    taowin::ComboBox* _c;
};

void test_combobox()
{
    auto w = new TW;
    w->create();
    w->show();
}
