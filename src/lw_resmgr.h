#pragma once

#include <map>
#include <string>

#include "lw_util.h"

namespace laywin{
	class resmgr{
        typedef std::map<string, HFONT> font_maps_t;

	public:
		resmgr();
		~resmgr();

		void    add_font(const char* name, const char* face, int size);
        HFONT   get_font(const char* name) {
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

	protected:
		HWND        _hwnd;
		font_maps_t _fonts;
	};
}
