#ifndef __laywin_syscontrols_h__
#define __laywin_syscontrols_h__

#include <vector>
#include <limits.h>

#include "lw_util.h"

namespace laywin{
	class syscontrol : public control
	{
	public:
		syscontrol();

	public:
		bool set_window_text(LPCTSTR str)
		{
			return !!::SetWindowText(_hwnd, str);
		}

		string get_window_text()
		{
			int len = ::GetWindowTextLength(_hwnd);
			char* p = new char[len + 1];
			p[::GetWindowText(_hwnd, p, len+1)] = '\0';

			string s(p);
			delete[] p;
			return s;
		}

        void create(HWND parent) {
            _hwnd = ::CreateWindowEx(_dwExStyle, get_control_class(), "", WS_CHILD | WS_VISIBLE | _dwStyle |WS_TABSTOP, 0, 0, 0, 0, parent, nullptr, nullptr, nullptr);
        }

	protected:
		virtual void init() override;
		virtual LPCTSTR get_control_class() const = 0;
		virtual void set_attr(const char* name, const char* value) override;
		virtual void set_style(std::vector<string>& styles, bool bex = false);

	protected:
		DWORD _dwStyle;
		DWORD _dwExStyle;
	};

	class button : public syscontrol
	{
	protected:
		virtual LPCTSTR get_control_class() const;
		virtual void set_style(std::vector<string>& styles, bool bex = false) override;
	};

	class option : public syscontrol
	{
	public:
		option();

	protected:
		virtual LPCTSTR get_control_class() const;
		virtual void set_style(std::vector<string>& styles, bool bex = false) override;

	protected:
		bool _b_has_group;
	};

	class check : public syscontrol
	{
	public:
		check();

	protected:
		virtual void init() override;
		virtual LPCTSTR get_control_class() const override;
		virtual void set_attr(const char* name, const char* value) override;

	protected:
		bool _b_check;
	};

	class static_ : public syscontrol
	{
	protected:
		virtual LPCTSTR get_control_class() const override;
	};

	class group : public syscontrol
	{
	public:
		group();

	protected:
		virtual LPCTSTR get_control_class() const override;
	};

	class edit : public syscontrol
	{
	public:
		edit();

	protected:
		virtual LPCTSTR get_control_class() const override;
		virtual void set_style(std::vector<string>& styles, bool bex /* = false */);
	};

	class listview : public syscontrol
	{
	public:
		listview();
		virtual void init() override;
		int insert_column(LPCTSTR name, int cx, int i);
		int insert_item(LPCTSTR str, LPARAM param = 0, int i = INT_MAX);
		bool delete_item(int i);
		int set_item(LPCTSTR str, int i, int isub);
		LPARAM get_param(int i, int isub);
		void format_columns(const string& fmt);
		int size() const;

	protected:
		virtual LPCTSTR get_control_class() const override;
		virtual void set_style(std::vector<string>& styles, bool bex /* = false */);

	private:
		string _fmt_column;
	};
}

#endif//__laywin_syscontrols_h__
