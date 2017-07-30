#pragma once

#include <string>
#include <functional>

#include <assert.h>
#include <windows.h>
#include "tw_util.h"

namespace taowin{

// 自适应的标准字符串版本
#ifdef _UNICODE
    typedef std::wstring string;
#else
    typedef std::string string;
#endif

    // 窗口消息过滤器，由消息循环调用
	class IMessageFilter{
	public:
        // 返回此窗口的窗口句柄
        virtual HWND filter_hwnd() = 0;

        // 过滤消息，如果被过滤，返回真
        // 那么消息循环不再分发此消息
		virtual bool filter_message(MSG* msg) = 0;
	};

    // 消息循环执行者
	class WindowManager{
	public:
        WindowManager() {}
        ~WindowManager() {}

        // 跑消息循环
        int loop_message()
        {
            MSG msg;

            for (;;) {
				// 如果有异步回调消息，就先处理
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

        // 执行消息过滤
        bool filter_message(MSG* msg) {
            if(_message_filters.empty())
                return false;

            // 来自子窗口的消息应该转发给其顶级窗口
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

        // 添加一个消息过滤器
        void add_message_filter(IMessageFilter* filter) {
            assert(_message_filters.find(filter) == -1);
            _message_filters.add(filter);
        }

        // 移除一个消息过滤器
        void remove_message_filter(IMessageFilter* filter) {
            assert(!_message_filters.empty());
            _message_filters.remove(filter);
            if(_message_filters.empty())
                quit(0);
        }

        // 结束消息循环
        void quit(int code = 0) {
            ::PostQuitMessage(code);
        }

        // 添加异步调用过程
        void add_async_call(std::function<void()> fn) {
            _async_calls.push_back(fn);
        }

        // 添加空闲处理过程
        void add_idle_handler(std::function<void()> fn) {
            _idle_handlers.push_back(fn);
        }

    protected:
        // 处理空闲调用
        void handle_idle() {
            if (!_idle_handlers.empty()) {
                auto handlers = std::move(_idle_handlers);

                for (auto& handler : handlers)
                    handler();
            }
        }

        // 处理异步调用
        void handle_async_call() {
            if (!_async_calls.empty()) {
                auto calls = std::move(_async_calls);

                for (auto& call : calls)
                    call();
            }
        }

	private:
		Array<IMessageFilter*>              _message_filters;   // 消息过滤器数组
        std::vector<std::function<void()>>  _idle_handlers;     // 空闲调用过程
		std::vector<std::function<void()>>  _async_calls;       // 异步调用过程
	};

    // （全局）主消息循环
    extern WindowManager __window_manager;

    // 注册全局窗口类
    void register_window_classes();

    struct WindowExtra {

    };

    // 窗口基类
    class Window
		: public IMessageFilter
	{
    protected:
        // 窗口标志
        struct WindowFlag {
            // 窗口创建后居中
            static const DWORD center = 1;
        };

        // 窗口创建时需要的一些元素
        struct WindowMeta {
            const TCHAR*    caption;    // 窗口标题
            const TCHAR*    classname;  // 窗口类名
            DWORD           style;      // 窗口风格
            DWORD           exstyle;    // 窗口扩展风格
            DWORD           flags;      // 窗口标志
        };

	public:
		Window();
		virtual ~Window();

        // 返回当前窗口的窗口句柄
		HWND hwnd() const { return _hwnd; }

        // 自动转换为窗口句柄
		operator HWND() const { return hwnd(); }

        // 创建此窗口，可以指定窗口的所有者
        HWND create(HWND owner = nullptr);
        HWND create(Window* w) { return create(w->hwnd()); }

        // 创建模态窗口
        int  domodal(HWND owner = nullptr);
        int  domodal(Window* w) { return domodal(*w); }

        // 关闭窗口（发送关闭消息，非销毁）
		void close(int code = 0);

        // 显示窗口
		void show(bool show = true, bool focus = true);

        // 居中窗口
		void center();

        // 弹出消息框
        int msgbox(const TCHAR* text, int type = (int)MB_OK, const TCHAR* title = _T("")) {
            return ::MessageBox(_hwnd, text, title, (UINT)type);
        }

        int msgbox(const string& text, int type = (int)MB_OK, const string& title = _T("")) {
            return msgbox(text.c_str(), type, title.c_str());
        }

        // 给自身发送窗口消息
		LRESULT send_message(UINT umsg, WPARAM wparam = 0, LPARAM lparam = 0){
			return ::SendMessage(_hwnd, umsg, wparam, lparam);
		}

        // 给自身投递窗口消息
		LRESULT post_message(UINT umsg, WPARAM wparam = 0, LPARAM lparam = 0){
			return ::PostMessage(_hwnd, umsg, wparam, lparam);
		}

        // 接口：返回窗口句柄
        virtual HWND filter_hwnd() override {
            return _hwnd;
        }

        // 接口：消息过滤
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

        // 过滤特殊键
        virtual bool filter_special_key(int vk) {
            if (vk == VK_ESCAPE) {
                close(-1);
                return true;
            }

            return false;
        }

        // 获取窗口矩形
        Rect get_window_rect() const {
            assert(_hwnd);
            Rect rc;
            ::GetWindowRect(_hwnd, &rc);
            return rc;
        }

        // 获取客户区矩形
        Rect get_client_Rect() const {
            assert(_hwnd);
            Rect rc;
            ::GetClientRect(_hwnd, &rc);
            return rc;
        }

    protected:
        // 获取窗口初始化元信息
        virtual void get_metas(WindowMeta* metas);

    private:
        friend void register_window_classes();

        // 全局窗口过程：分发消息给各窗口的 __handle_message 过程
        static LRESULT __stdcall __window_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	protected:
        // 窗口实例的消息处理点
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

        // 收到第一条消息时调用
		virtual void on_first_message();

        // 收到最后一条消息时调用
		virtual void on_final_message();

	protected:
		HWND    _hwnd;          // 窗口句柄
        int     _return_code;   // 返回码
	};
}
