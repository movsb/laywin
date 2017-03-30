#include <taowin/core/tw_syscontrols.h>
#include "header.h"

namespace taowin {

void HeaderControl::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
{
    metas.classname = WC_HEADER;
}

bool HeaderControl::filter_notify(int code, NMHDR* hdr, LRESULT* lr)
{
    if(code == NM_RCLICK) {
        if(on_rclick) {
            *lr = on_rclick();
            return true;
        }
    }
    else if(code == HDN_ENDTRACK){
        if(on_end_track) {
            *lr = on_end_track(hdr);
            return true;
        }
    }
    else if(code == HDN_DIVIDERDBLCLICK) {
        if(on_divider_dblclick) {
            *lr = on_divider_dblclick(hdr);
            return true;
        }
    }
    else if(code == HDN_ENDDRAG) {
        if(on_end_drag) {
            *lr = on_end_drag();
            return true;
        }
    }

    return false;
}

}