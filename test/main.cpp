#include <Windows.h>
#include <taowin/core/tw_taowin.h>

/*
#ifdef TAOLOG_ENABLED
// {2E51B67B-565A-4609-AE11-8979DCD4687F}
static const GUID providerGuid =
{ 0x2E51B67B, 0x565A, 0x4609, { 0xAE, 0x11, 0x89, 0x79, 0xDC, 0xD4, 0x68, 0x7F } };
TaoLogger g_taoLogger(providerGuid);
#endif
*/

#define pmain test_button


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    taowin::init();

    extern void pmain();
    pmain();

    taowin::loop_message();
	
	return 0;
}
