#include <windows.h>
#include "tw_util.h"
#include "tw_resmgr.h"

namespace taowin{

	ResourceManager::ResourceManager()
        : _hwnd(nullptr)
        , _next_ctrl_id(1)
	{
	}

	ResourceManager::~ResourceManager()
	{
        // TODO remove all fonts
	}

	void ResourceManager::add_font(const TCHAR* name, const TCHAR* face, int size)
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
