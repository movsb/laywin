#pragma once

#include <map>
#include <string>

#include "tw_util.h"

namespace taowin{
    class WindowCreator;
    class Control;
    class SystemControl;

    class ResourceManager {
    public:
        typedef std::map<string, HFONT> FontMap;

        friend class WindowCreator;
        friend class Control;
        friend class SystemControl;

	public:
		ResourceManager();
		~ResourceManager();

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

        unsigned int next_ctrl_id()
        {
            return _next_ctrl_id++;
        }

	private:
		HWND        _hwnd;
        FontMap _fonts;
        WindowCreator* _owner;
        unsigned int    _next_ctrl_id;
	};
}
