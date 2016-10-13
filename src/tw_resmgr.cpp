#include <windows.h>
#include "tw_util.h"
#include "tw_resmgr.h"

namespace taowin{

	resmgr::resmgr()
        : _hwnd(nullptr)
	{
	}

	resmgr::~resmgr()
	{
        // TODO remove all fonts
	}

	void resmgr::add_font(const TCHAR* name, const TCHAR* face, int size)
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
