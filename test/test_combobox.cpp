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
            *tag =  reinterpret_cast<void*>(index * index);
        }

        DataSource()
        {
            _strs.emplace_back(_T("1"));
            _strs.emplace_back(_T("2"));
            _strs.emplace_back(_T("3"));
            _strs.emplace_back(_T("4"));
            _strs.emplace_back(_T("5"));
            _strs.emplace_back(_T("6"));
            _strs.emplace_back(_T("7"));
            _strs.emplace_back(_T("8"));
        }

    protected:
        std::vector<const TCHAR*> _strs;
    };

protected:
	virtual LPCTSTR get_skin_xml() const override
	{
        LPCTSTR json = _T(R"tw(
<Window title="taowinÑÝÊ¾´°¿Ú" size="500,300">
    <Resource>
        <Font name="default" face="Î¢ÈíÑÅºÚ" size="12"/>
        <Font name="1" face="Î¢ÈíÑÅºÚ" size="12"/>
    </Resource>
    <Root>
        <Vertical padding="5,5,5,5">
            <ComboBox name="c" />
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
                LogLog(_T("Ñ¡ÖÐ¸Ä±ä %d,%p"), index, ud);
                return 0;
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
