#pragma once

#include <string>
#include <functional>

#include <assert.h>
#include <windows.h>
#include "tw_util.h"

namespace taowin{
    
#ifdef _UNICODE
    typedef std::wstring string;
#else
    typedef std::string string;
#endif

	class IMessageFilter{
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

            for (;;) {
                if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT)
                        break;

                    if (!filter_message(&msg)) {
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                    }
                }
                else if (_idle_handlers.empty()) {
                    ::GetMessage(&msg, nullptr, 0, 0);
                    if (msg.message == WM_QUIT)
                        break;

                    if (!filter_message(&msg)) {
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                    }
                }
                else {
                    handle_idle();
                }
            }

            return (int)msg.wParam;
        }

        bool filter_message(MSG* msg) {
            if(_message_filters.empty())
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

        void add_message_filter(IMessageFilter* filter) {
            assert(_message_filters.find(filter) == -1);
            _message_filters.add(filter);
        }

        void remove_message_filter(IMessageFilter* filter) {
            assert(!_message_filters.empty());
            _message_filters.remove(filter);
            if(_message_filters.empty())
                quit(0);
        }

        void quit(int code = 0) {
            ::PostQuitMessage(code);
        }

        void add_async_call(std::function<void()> fn) {
            _idle_handlers.push_back(fn);
        }

    protected:
        void handle_idle() {
            if (!_idle_handlers.empty()) {
                auto handlers = std::move(_idle_handlers);

                for (auto& handler : handlers)
                    handler();
            }
        }

	private:
		array<IMessageFilter*>              _message_filters;
        std::vector<std::function<void()>>  _idle_handlers;
	};

    extern window_manager __window_manager;

    void register_window_classes();

    struct window_extra_t {

    };

    class window
		: public IMessageFilter
	{
    protected:
        struct WindowFlag {
            static const DWORD center = 1;
        };

        struct WindowMeta {
            const TCHAR*    caption;
            const TCHAR*    classname;
            DWORD           style;
            DWORD           exstyle;
            DWORD           flags;
        };

	public:
		window();
		virtual ~window();

		HWND hwnd() const { return _hwnd; }
		operator HWND() const { return hwnd(); }

        HWND create(HWND owner = nullptr);
        HWND create(window* w) { return create(w->hwnd()); }

        int  domodal(HWND owner = nullptr);
        int  domodal(window* w) { return domodal(*w); }

		void close(int code = 0);

		void show(bool show = true, bool focus = true);
		void center();

        int msgbox(const TCHAR* text, int type = (int)MB_OK, const TCHAR* title = _T("")) {
            return ::MessageBox(_hwnd, text, title, (UINT)type);
        }

        int msgbox(const string& text, int type = (int)MB_OK, const string& title = _T("")) {
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

        virtual bool filter_special_key(int vk) {
            if (vk == VK_ESCAPE) {
                close(-1);
                return true;
            }

            return false;
        }

		virtual bool filter_message(MSG* msg) override {
			if(msg->message == WM_KEYDOWN){
				switch(msg->wParam)
				{
                case VK_ESCAPE:
                case VK_TAB:
                case VK_RETURN:
                    if (filter_special_key(msg->wParam))
                        return true;
                    // pass through
                default:
                    // I don't want IsDialogMessage to process VK_ESCAPE, because it produces a WM_COMMAND
                    // menu message with id == 2. It is undocumented.
                    if(::IsDialogMessage(_hwnd, msg))
                        return true;
                    break;
				}
			}
			return false;
		}

        Rect get_window_Rect() const {
            assert(_hwnd);
            Rect rc;
            ::GetWindowRect(_hwnd, &rc);
            return rc;
        }

        Rect get_client_Rect() const {
            assert(_hwnd);
            Rect rc;
            ::GetClientRect(_hwnd, &rc);
            return rc;
        }
    protected:
        virtual void get_metas(WindowMeta* metas);

    private:
        friend void register_window_classes();
		static LRESULT __stdcall __window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	protected:
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);
		virtual void on_first_message();
		virtual void on_final_message();

	protected:
		HWND    _hwnd;
        int     _return_code;
	};
}
