#include <core/tw_syscontrols.h>
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

    return false;
}

}