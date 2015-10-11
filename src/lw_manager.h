#ifndef __laywin_manager_h__
#define __laywin_manager_h__

#include <map>
#include <string>

namespace laywin{
	class manager{
	public:
		manager();
		~manager();

		HFONT default_font() const{
			return _default_font;
		}

		void default_font(LPCTSTR name, int size, bool bold, bool underline, bool italic);
		HFONT add_font(LPCTSTR name, LPCTSTR face, int size, bool bold, bool underline, bool italic);
		void remove_all_fonts();
		void hwnd(HWND h){
			_hwnd = h;
		}
		HWND hwnd() const{
			return _hwnd;
		}

		csize init_size() const {
			return _init_size;
		}

	protected:
		HWND _hwnd;
		HFONT _default_font;
		std::map<std::string, HFONT> _fonts;
		csize _init_size;
	};
}

#endif //__laywin_manager_h__
