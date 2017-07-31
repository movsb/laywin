#pragma once

#include <string>
#include <functional>

#include <assert.h>
#include <windows.h>
#include "tw_util.h"

namespace taowin{

// ����Ӧ�ı�׼�ַ����汾
#ifdef _UNICODE
    typedef std::wstring string;
#else
    typedef std::string string;
#endif

    // ������Ϣ������������Ϣѭ������
	class IMessageFilter{
	public:
        // ���ش˴��ڵĴ��ھ��
        virtual HWND filter_hwnd() = 0;

        // ������Ϣ����������ˣ�������
        // ��ô��Ϣѭ�����ٷַ�����Ϣ
		virtual bool filter_message(MSG* msg) = 0;
	};

    // ��Ϣѭ��ִ����
	class WindowManager{
	public:
        WindowManager() {}
        ~WindowManager() {}

        // ����Ϣѭ��
        int loop_message()
        {
            MSG msg;

            for (;;) {
				// ������첽�ص���Ϣ�����ȴ���
				if(!_async_calls.empty()) {
					handle_async_call();
				}

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

        // ִ����Ϣ����
        bool filter_message(MSG* msg) {
            if(_message_filters.empty())
                return false;

            // �����Ӵ��ڵ���ϢӦ��ת�����䶥������
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

        // ���һ����Ϣ������
        void add_message_filter(IMessageFilter* filter) {
            assert(_message_filters.find(filter) == -1);
            _message_filters.add(filter);
        }

        // �Ƴ�һ����Ϣ������
        void remove_message_filter(IMessageFilter* filter) {
            assert(!_message_filters.empty());
            _message_filters.remove(filter);
            if(_message_filters.empty())
                quit(0);
        }

        // ������Ϣѭ��
        void quit(int code = 0) {
            ::PostQuitMessage(code);
        }

        // ����첽���ù���
        void add_async_call(std::function<void()> fn) {
            _async_calls.push_back(fn);
        }

        // ��ӿ��д������
        void add_idle_handler(std::function<void()> fn) {
            _idle_handlers.push_back(fn);
        }

    protected:
        // ������е���
        void handle_idle() {
            if (!_idle_handlers.empty()) {
                auto handlers = std::move(_idle_handlers);

                for (auto& handler : handlers)
                    handler();
            }
        }

        // �����첽����
        void handle_async_call() {
            if (!_async_calls.empty()) {
                auto calls = std::move(_async_calls);

                for (auto& call : calls)
                    call();
            }
        }

	private:
		Array<IMessageFilter*>              _message_filters;   // ��Ϣ����������
        std::vector<std::function<void()>>  _idle_handlers;     // ���е��ù���
		std::vector<std::function<void()>>  _async_calls;       // �첽���ù���
	};

    // ��ȫ�֣�����Ϣѭ��
    extern WindowManager __window_manager;

    // ע��ȫ�ִ�����
    void register_window_classes();

    struct WindowExtra {

    };

    // ���ڻ���
    class Window
		: public IMessageFilter
	{
    protected:
        // ���ڱ�־
        struct WindowFlag {
            // ���ڴ��������
            static const DWORD center = 1;
        };

        // ���ڴ���ʱ��Ҫ��һЩԪ��
        struct WindowMeta {
            const TCHAR*    caption;    // ���ڱ���
            const TCHAR*    classname;  // ��������
            DWORD           style;      // ���ڷ��
            DWORD           exstyle;    // ������չ���
            DWORD           flags;      // ���ڱ�־
        };

	public:
		Window();
		virtual ~Window();

        // ���ص�ǰ���ڵĴ��ھ��
		HWND hwnd() const { return _hwnd; }

        // �Զ�ת��Ϊ���ھ��
		operator HWND() const { return hwnd(); }

        // �����˴��ڣ�����ָ�����ڵ�������
        HWND create(HWND owner = nullptr);
        HWND create(Window* w) { return create(w->hwnd()); }

        // ����ģ̬����
        int  domodal(HWND owner = nullptr);
        int  domodal(Window* w) { return domodal(*w); }

        // �رմ��ڣ����͹ر���Ϣ�������٣�
		void close(int code = 0);

        // ��ʾ����
		void show(bool show = true, bool focus = true);

        // ���д���
		void center();

        // ������Ϣ��
        int msgbox(const TCHAR* text, int type = (int)MB_OK, const TCHAR* title = _T("")) {
            return ::MessageBox(_hwnd, text, title, (UINT)type);
        }

        int msgbox(const string& text, int type = (int)MB_OK, const string& title = _T("")) {
            return msgbox(text.c_str(), type, title.c_str());
        }

        // �������ʹ�����Ϣ
		LRESULT send_message(UINT umsg, WPARAM wparam = 0, LPARAM lparam = 0){
			return ::SendMessage(_hwnd, umsg, wparam, lparam);
		}

        // ������Ͷ�ݴ�����Ϣ
		LRESULT post_message(UINT umsg, WPARAM wparam = 0, LPARAM lparam = 0){
			return ::PostMessage(_hwnd, umsg, wparam, lparam);
		}

        // �ӿڣ����ش��ھ��
        virtual HWND filter_hwnd() override {
            return _hwnd;
        }

        // �ӿڣ���Ϣ����
		virtual bool filter_message(MSG* msg) override {
			if(msg->message == WM_KEYDOWN){
				switch(msg->wParam)
				{
                case VK_ESCAPE:
                case VK_RETURN:
                    if (filter_special_key(msg->wParam))
                        return true;
                    break;
                case VK_TAB:
                    // I don't want IsDialogMessage to process VK_ESCAPE, because it produces a WM_COMMAND
                    // menu message with id == 2. It is undocumented.
                    if(::IsDialogMessage(_hwnd, msg))
                        return true;
                    break;
				}
			}
			return false;
		}

        // ���������
        virtual bool filter_special_key(int vk) {
            if (vk == VK_ESCAPE) {
                close(-1);
                return true;
            }

            return false;
        }

        // ��ȡ���ھ���
        Rect get_window_rect() const {
            assert(_hwnd);
            Rect rc;
            ::GetWindowRect(_hwnd, &rc);
            return rc;
        }

        // ��ȡ�ͻ�������
        Rect get_client_Rect() const {
            assert(_hwnd);
            Rect rc;
            ::GetClientRect(_hwnd, &rc);
            return rc;
        }

    protected:
        // ��ȡ���ڳ�ʼ��Ԫ��Ϣ
        virtual void get_metas(WindowMeta* metas);

    private:
        friend void register_window_classes();

        // ȫ�ִ��ڹ��̣��ַ���Ϣ�������ڵ� __handle_message ����
        static LRESULT __stdcall __window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	protected:
        // ����ʵ������Ϣ�����
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

        // �յ���һ����Ϣʱ����
		virtual void on_first_message();

        // �յ����һ����Ϣʱ����
		virtual void on_final_message();

	protected:
		HWND    _hwnd;          // ���ھ��
        int     _return_code;   // ������
	};
}
