#pragma once

namespace taowin {

class Progress : public SystemControl
{
public:
    void set_range(int min, int max);
    void set_pos(int pos);
    void set_bkcolor(COLORREF color);
    void set_color(COLORREF color);

protected:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
    virtual void set_attr(const TCHAR* name, const TCHAR* value) override;
};


}
