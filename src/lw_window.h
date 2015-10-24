#pragma once

#include <assert.h>
#include <windows.h>
#include "lw_util.h"

namespace laywin{
	class i_message_filter{
	public:
        virtual HWND filter_hwnd() = 0;
		virtual bool filter_message(MSG* msg) = 0;
	};

	class window_manager{
	public:
        window_manager() {}
        ~window_manager() {}

        void loop_message()
        {
            MSG msg;
            while(::GetMessage(&msg, NULL, 0, 0)){
                if(!filter_message(&msg)){
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
            }
        }

        bool filter_message(MSG* msg) {
            if(_message_filters.size() == 0)
                return false;

            if(msg->message == WM_KEYDOWN) {
                msg = msg;
            }
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
            _message_filters.add(filter);
        }

        void remove_message_filter(i_message_filter* filter) {
            _message_filters.remove(filter);
        }

        void quit(int code = 0) {
            ::PostQuitMessage(code);
        }

	public:
		static array<i_message_filter*> _message_filters;
	};

    class dialog_manager{
    public:
        dialog_manager(i_message_filter*  this_) {
            _this = this_;
            _hwnd = _this->filter_hwnd();
        }

        int run() {
            MSG msg;
            while(::GetMessage(&msg, NULL, 0, 0)) {
                if(!filter_message(&msg)) {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
            }

            if(msg.message == WM_QUIT)
                return (int)msg.wParam;

            return 0x80000000;
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
    };

    void register_window_classes();

    struct window_extra_t {

    };

	class window
		: public i_message_filter
	{
        struct window_meta_t {
            const char*     caption;
            const char*     classname;
            DWORD           style;
            DWORD           exstyle;

            window_meta_t(const char* caption_ = "", const char* classname_ = "",
                DWORD style_ = WS_OVERLAPPEDWINDOW, DWORD exstyle_ = 0) 
            {
                caption = caption_;
                classname = classname_;
                style = style_;
                exstyle = exstyle_;
            }
        };

	public:
		window();
		virtual ~window();

		HWND hwnd() const { return _hwnd; }
		operator HWND() const { return hwnd(); }

        HWND create(const window_meta_t& meta);
        int  domodal(const window_meta_t& meta, HWND owner=nullptr);
		void close();
		void show(bool show = true, bool focus = true);
		void center();

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

    public:
		static LRESULT __stdcall __window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	protected:
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled);
		virtual void on_first_message();
		virtual void on_final_message();

	protected:
		HWND    _hwnd;
        bool    _is_dialog;
	};
}
