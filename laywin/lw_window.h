#ifndef __laywin_window_h__
#define __laywin_window_h__

#include <assert.h>
#include <windows.h>
#include "lw_util.h"

namespace laywin{
	class i_message_filter{
	public:
		virtual bool filter_message(HWND hChild, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	};

	class i_accelerator_translator{
	public:
		virtual bool translate_accelerator(MSG* pmsg) = 0;
	};

	class window_manager{
	public:
		window_manager();
		virtual ~window_manager();

		HWND hwnd() { return _hwnd; }

		void init(HWND hwnd, i_message_filter* flt, i_accelerator_translator* trans);
		void deinit();

		bool filter_message(MSG* pmsg);
		i_message_filter* message_filter() const { return _message_filter; }
		void message_filter(i_message_filter* flt) { _message_filter = flt; }

		bool translate_accelerator(MSG* pmsg);
		i_accelerator_translator* accelerator_translator() const { return _accelerator_translator; }
		void accelerator_translator(i_accelerator_translator* trans) { _accelerator_translator = trans; }

		static void message_loop();
		static bool translate_meesage(MSG* pmsg);

		bool add_window_manager(window_manager* pwm){
			return _window_managers.add(pwm);
		}
		bool remove_window_manager(window_manager* pwm){
			bool b = _window_managers.remove(pwm);
			if(_window_managers.size() == 0){
				::PostQuitMessage(0);
			}
			return b;
		}

	protected:
		HWND _hwnd;
		i_message_filter* _message_filter;
		i_accelerator_translator* _accelerator_translator;

		static array<window_manager*> _window_managers;
	};

	class window
		: public i_message_filter
		, public i_accelerator_translator
	{
	public:
		enum class window_type{
			normal,
			modal_dialog,
			modeless_dialog,
		};
	public:
		window();
		virtual ~window();

		HWND hwnd() const { return _hwnd; }
		operator HWND() const { return hwnd(); }

		HWND create(HWND hParent=NULL, DWORD dwStyle=0, DWORD dwExStyle=0, HMENU hMenu = NULL);
		void close();

		bool do_modal(HWND owner);
		bool do_modeless(HWND owner);

		void show(bool show = true, bool focus = true);
		void center();

		LRESULT send_message(UINT umsg, WPARAM wparam = 0, LPARAM lparam = 0){
			return ::SendMessage(_hwnd, umsg, wparam, lparam);
		}
		LRESULT post_message(UINT umsg, WPARAM wparam = 0, LPARAM lparam = 0){
			return ::PostMessage(_hwnd, umsg, wparam, lparam);
		}

		virtual bool filter_message(HWND hChild, UINT uMsg, WPARAM wParam, LPARAM lParam){
			if(uMsg == WM_KEYDOWN){
				switch(wParam)
				{
				case VK_RETURN:
				case VK_ESCAPE:
					return response_default_key_event(hChild, wParam);
				default:
					break;
				}
			}
			return false;
		}

		virtual bool translate_accelerator(MSG* pmsg){
			return false;
		}

	protected:
		bool register_window_class();
		bool register_super_class();
		HWND subclass(HWND hwnd);
		void unsubclass();

		virtual LPCTSTR get_window_class_name() const { return _T("laywin_class"); }
		virtual LPCTSTR get_super_class_name() const { return NULL; }
		virtual UINT get_class_style() const{ return CS_HREDRAW | CS_VREDRAW; }
		virtual HBRUSH get_class_brush() const{ return (HBRUSH)(COLOR_WINDOW); }
		virtual DWORD get_window_style() const{ return WS_OVERLAPPEDWINDOW; }
		virtual DWORD get_window_extended_style(){ return 0; }
		virtual LPCTSTR get_window_name() const{ return _T("laywin_window"); }
		
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled);
		virtual void on_first_message(HWND hwnd);
		virtual void on_final_message(HWND hwnd);
		virtual bool response_default_key_event(HWND hChild, WPARAM wParam);

		static LRESULT __stdcall __window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
		static LRESULT __stdcall __control_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
		static INT_PTR __stdcall __dialog_procedure(HWND  hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	protected:
		HWND _hwnd;
		window_type _type;
		WNDPROC _old_wnd_proc;
		bool _b_subclassed;
		window_manager _wndmgr;
		bool _b_has_mgr;
	};
}

#endif//__laywin_window_h__
