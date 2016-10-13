#pragma once

#include <map>
#include <string>

#include "tw_util.h"

namespace taowin{
    class resmgr {
    public:
        typedef std::map<string, HFONT> font_maps_t;

	public:
		resmgr();
		~resmgr();

		void    add_font(const TCHAR* name, const TCHAR* face, int size);
        HFONT   get_font(const TCHAR* name) {
            auto it = _fonts.find(name);
            if(it != _fonts.cend())
                return it->second;
            return nullptr;
        }
		void    set_hwnd(HWND h){
			_hwnd = h;
		}

        HWND    get_hwnd() const {
            return _hwnd;
        }

	private:
		HWND        _hwnd;
        font_maps_t _fonts;
	};
}
