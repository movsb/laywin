#pragma once

namespace taowin {

class HeaderControl : public SystemControl
{
public:
    enum ReturnValue
    {
        RClick_NotAllowDefault      = 1,
        RClick_AllowDefault         = 0,
    };

public:
    virtual void get_metas(SystemControlMetas& metas, std::map<string, string>& attrs) override;
    virtual bool filter_notify(int code, NMHDR* hdr, LRESULT* lr) override;

public:
    OnNotify        on_rclick;
    OnHdrNotify     on_end_track;
    OnHdrNotify     on_divider_dblclick;
    OnNotify        on_end_drag;
};


}