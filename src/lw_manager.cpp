#include <windows.h>
#include "lw_util.h"
#include "lw_manager.h"

namespace laywin{

	manager::manager()
	{
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		lf.lfCharSet = DEFAULT_CHARSET;
		_default_font = ::CreateFontIndirect(&lf);
		_init_size.cx = _init_size.cy = 500;
	}

	manager::~manager()
	{
		::DeleteObject(_default_font);
		remove_all_fonts();
	}

	void manager::default_font(LPCTSTR name, int size, bool bold, bool underline, bool italic)
	{
		if (_default_font) ::DeleteObject(_default_font);

		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		_tcsncpy(lf.lfFaceName, name, LF_FACESIZE);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -size;
		if (bold) lf.lfWeight += FW_BOLD;
		if (underline) lf.lfUnderline = TRUE;
		if (italic) lf.lfItalic = TRUE;
		_default_font = ::CreateFontIndirect(&lf);
	}

	HFONT manager::add_font(LPCTSTR name, int size, bool bold, bool underline, bool italic)
	{
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		_tcsncpy(lf.lfFaceName, name, LF_FACESIZE);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -size;
		if (bold) lf.lfWeight += FW_BOLD;
		if (underline) lf.lfUnderline = TRUE;
		if (italic) lf.lfItalic = TRUE;
		HFONT hFont = ::CreateFontIndirect(&lf);
		if (hFont == NULL) return NULL;

		if (!_fonts.add(hFont)){
			::DeleteObject(hFont);
			return NULL;
		}
		return hFont;
	}

	bool manager::remove_font(HFONT hFont)
	{
		for (int i = 0; i < _fonts.size(); i++){
			if(_fonts[i] == hFont){
				::DeleteObject(hFont);
				return _fonts.remove(i);
			}
		}
		return false;
	}

	void manager::remove_all_fonts()
	{
		for (int i = 0; i < _fonts.size(); i++){
			::DeleteObject(_fonts[i]);
		}
		_fonts.empty();
	}

}
