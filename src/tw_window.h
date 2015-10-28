#pragma once

#include <assert.h>
#include <windows.h>
#include "tw_util.h"

namespace taowin{
	class i_message_filter{
	public:
        virtual HWND filter_hwnd() = 0;
		virtual bool filter_message(MSG* msg) = 0;
	};

	class window_manager{
	public:
        window_manager() {}
        ~window_manager() {}

        int loop_message()
        {
            MSG msg;
            while(::GetMessage(&msg, NULL, 0, 0)){
                if(!filter_message(&msg)){
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
            }

            return (int)msg.wParam;
        }

        bool filter_message(MSG* msg) {
            if(_message_filters.size() == 0)
                return false;

            HWND parent = msg->hwnd;
            while(parent && ::GetWindowLongPtr(parent, GWL_STYLE) & WS_CHILD)
                parent = ::GetParent(parent);

            for(int i = 0; i < _message_filters.size(); i++) {
                auto flt = _message_filters[i];
                if(parent == flt->filter_hwnd() && flt->filter_message(msg)) {
                    return true;
                }
            }
            return false;
        }

        void add_message_filter(i_message_filter* filter) {
            assert(_message_filters.find(filter) == -1);
            _message_filters.add(filter);
        }

        void remove_message_filter(i_message_filter* filter) {
            assert(_message_filters.size() > 0);
            _message_filters.remove(filter);
            if(_message_filters.size() == 0)
                quit(0);
        }

        void quit(int code = 0) {
            ::PostQuitMessage(code);
        }

	private:
		array<i_message_filter*> _message_filters;
	};

    extern window_manager __window_manager;

    class dialog_manager{
    public:
        dialog_manager(i_message_filter*  this_, HWND owner) {
            _this = this_;
            _hwnd = _this->filter_hwnd();
            _owner = owner;
        }

        int run() {
            assert(::IsWindow(_owner));
            ::EnableWindow(_owner, FALSE);
            MSG msg;
            while(::GetMessage(&msg, NULL, 0, 0)) {
                if(!filter_message(&msg)) {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
            }

            return (int)msg.wParam;
        }
    private:
        bool filter_message(MSG* msg) {
            HWND parent = msg->hwnd;
            while(parent && ::GetWindowLongPtr(parent, GWL_STYLE) & WS_CHILD)
                parent = ::GetParent(parent);

            return _hwnd == parent && _this->filter_message(msg);
        }
    private:
        i_message_filter*   _this;
        HWND                _hwnd;
        HWND                _owner;
    };

    void register_window_classes();

    struct window_extra_t {

    };

    class window
		: public i_message_filter
	{
    protected:
        struct window_meta_t {
            const char*     caption;
            const char*     classname;
            DWORD           style;
            DWORD           exstyle;
        };

	public:
		window();
		virtual ~window();

		HWND hwnd() const { return _hwnd; }
		operator HWND() const { return hwnd(); }

        HWND create();
        int  domodal(HWND owner=nullptr);
        int  domodal(window* w) { return domodal(*w); }
		void close(int code = 0);
		void show(bool show = true, bool focus = true);
		void center();

        int msgbox(const char* text, int type = (int)MB_OK, const char* title = "") {
            return ::MessageBox(_hwnd, text, title, (UINT)type);
        }
        int msgbox(const std::string& text, int type = (int)MB_OK, const std::string& title = "") {
            return msgbox(text.c_str(), type, title.c_str());
        }

		LRESULT send_message(UINT umsg, WPARAM wparam = 0, LPARAM lparam = 0){
			return ::SendMessage(_hwnd, umsg, wparam, lparam);
		}
		LRESULT post_message(UINT umsg, WPARAM wparam = 0, LPARAM lparam = 0){
			return ::PostMessage(_hwnd, umsg, wparam, lparam);
		}

        virtual HWND filter_hwnd() override {
            return _hwnd;
        }

		virtual bool filter_message(MSG* msg) override {
			if(msg->message == WM_KEYDOWN){
				switch(msg->wParam)
				{
                case VK_ESCAPE:
                    close();
                    return true;
                default:
                    // I don't want IsDialogMessage to process VK_ESCAPE, because it produces a WM_COMMAND
                    // menu message with id == 2. It is undocumented.
                    // and, this function call doesn't care the variable _is_dialog.
                    if(::IsDialogMessage(_hwnd, msg))
                        return true;
                    break;
				}
			}
			return false;
		}

    protected:
        virtual void get_metas(window_meta_t* metas);

    private:
        friend void register_window_classes();
		static LRESULT __stdcall __window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	protected:
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);
		virtual void on_first_message();
		virtual void on_final_message();

	protected:
		HWND    _hwnd;
        bool    _is_dialog;
        int     _return_code;
	};
}
