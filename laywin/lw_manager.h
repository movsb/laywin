#ifndef __laywin_manager_h__
#define __laywin_manager_h__

namespace laywin{
	class manager{
	public:
		manager();
		~manager();

		HFONT default_font() const{
			return _default_font;
		}

		void default_font(LPCTSTR name, int size, bool bold, bool underline, bool italic);
		HFONT add_font(LPCTSTR name, int size, bool bold, bool underline, bool italic);
		bool remove_font(HFONT hFont);
		void remove_all_fonts();
		HFONT font(int i) const{
			if (i<0 || i>_fonts.size()) return default_font();
			return _fonts[i];
		}
		int font(HFONT hFont){
			for (int i = 0; i < _fonts.size(); i++){
				if (_fonts[i] == hFont){
					return i;
				}
			}
			return -1;
		}

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
		array<HFONT> _fonts;
		csize _init_size;
	};
}

#endif //__laywin_manager_h__
