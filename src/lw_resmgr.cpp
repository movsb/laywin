#include <windows.h>
#include "lw_util.h"
#include "lw_resmgr.h"

namespace laywin{

	resmgr::resmgr()
        : _hwnd(nullptr)
	{
	}

	resmgr::~resmgr()
	{
        // TODO remove all fonts
	}

	void resmgr::add_font(const char* name, const char* face, int size)
	{
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		_tcsncpy(lf.lfFaceName, face, LF_FACESIZE);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -size;

		HFONT hFont = ::CreateFontIndirect(&lf);
        if(hFont != nullptr) {
            _fonts[name] = hFont;
        }
	}
}
