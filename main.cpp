#include <Windows.h>
#include "core/tw_taowin.h"

// {9CEC8287-B104-4A62-AF00-3DA900848A18}
static const GUID providerGUID = 
{ 0x9cec8287, 0xb104, 0x4a62, { 0xaf, 0x0, 0x3d, 0xa9, 0x0, 0x84, 0x8a, 0x18 } };
ETWLogger g_etwLogger(providerGUID);

#define pmain test_tabctrl


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    taowin::init();

    extern void pmain();
    pmain();

    taowin::loop_message();
	
	return 0;
}
