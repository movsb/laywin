#include <Windows.h>
#include "core/tw_taowin.h"

#define pmain test_webview


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    taowin::init();

    extern void pmain();
    pmain();

    taowin::loop_message();
	
	return 0;
}
