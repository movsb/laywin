#include <taowin/core/tw_syscontrols.h>
#include "TabCtrl.h"

namespace taowin {

int TabCtrl::insert_item(int i, const TCHAR* s, const void* p)
{
    TCITEM item = {0};
    item.mask = TCIF_TEXT | TCIF_PARAM;
    item.pszText = const_cast<TCHAR*>(s);
    item.lParam = (LPARAM)p;

    return TabCtrl_InsertItem(_hwnd, i, &item);
}

void TabCtrl::adjust_rect(bool large, RECT * rc)
{
    TabCtrl_AdjustRect(_hwnd, !!large, rc);
}

void TabCtrl::get_metas(SystemControlMetas& metas, std::map<string, string>& attrs)
{
    metas.classname = WC_TABCONTROL;
}


}