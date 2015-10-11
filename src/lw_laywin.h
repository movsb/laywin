#ifndef __laywin_h__
#define __laywin_h__

#include "lw_util.h"
#include "lw_control.h"
#include "lw_syscontrols.h"
#include "lw_window.h"
#include "lw_manager.h"

namespace laywin{
	class laywin{
	public:
		laywin();
		virtual ~laywin();

		void set_layout(LPCTSTR json, HWND hwnd);
		void set_size(int cx, int cy);
		container* root() const { return _root; }
		container* operator->(){ return _root; }

	public:
		container* _root;
		manager _mgr;
	};

	class window_creator : public window
	{
	public:
		virtual HWND create(HWND hParent=NULL, DWORD dwStyle=0, DWORD dwExStyle=0, HMENU hMenu = NULL);

	protected:
		virtual LPCTSTR get_skin_json() const;
		virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled);
		virtual void on_first_message(HWND hwnd) override;
		virtual void on_final_message(HWND hwnd) override;
		virtual LRESULT on_menu(int id) { return 0; }
		virtual LRESULT on_command_ctrl(HWND hwnd, control* pc, int code) { return 0; }
		virtual LRESULT on_notify_ctrl(HWND hwnd, control* pc, int code, NMHDR* hdr) { return 0; }

	private:
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled) override;

	protected:
		laywin _layout;
	};
}

#endif//__laywin_h__
