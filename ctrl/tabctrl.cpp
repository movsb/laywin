#include <core/tw_syscontrols.h>
#include "tabctrl.h"

namespace taowin {

int tabctrl::insert_item(int i, const TCHAR* s, const void* p)
{
    TCITEM item = {0};
    item.mask = TCIF_TEXT | TCIF_PARAM;
    item.pszText = const_cast<TCHAR*>(s);
    item.lParam = (LPARAM)p;

    return TabCtrl_InsertItem(_hwnd, i, &item);
}

void tabctrl::adjust_rect(bool large, RECT * rc)
{
    TabCtrl_AdjustRect(_hwnd, !!large, rc);
}

void tabctrl::get_metas(syscontrol_metas& metas, std::map<string, string>& attrs)
{
    metas.classname = WC_TABCONTROL;
}


}