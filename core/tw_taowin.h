#ifndef __taowin_h__
#define __taowin_h__

#include <functional>

#include <windows.h>
#include <CommCtrl.h>
#include <windowsx.h>

#include <taowin/core/tw_util.h>
#include <taowin/core/tw_control.h>
#include <taowin/core/tw_syscontrols.h>
#include <taowin/core/tw_window.h>
#include <taowin/core/tw_resmgr.h>
#include <taowin/core/tw_menu.h>

#include <taowin/ctrl/button.h>
#include <taowin/ctrl/combobox.h>
#include <taowin/ctrl/TextBox.h>
#include <taowin/ctrl/header.h>
#include <taowin/ctrl/listview.h>
#include <taowin/ctrl/Progress.h>
#include <taowin/ctrl/TabCtrl.h>
#include <taowin/ctrl/webview.h>

#include "../taolog.h"

namespace taowin{

    // 界面层的窗口基类
    class WindowCreator : public Window
	{
    public:
        WindowCreator();
        virtual ~WindowCreator();

        // 子类化某个系统控件
        void subclass_control(SystemControl* ctl);

        // 添加一个菜单，用于自动分发菜单消息
        void add_menu(const MenuItem* menu) { _menus.add(menu); }

	protected:
        // 获取皮肤 XML 文件
		virtual LPCTSTR get_skin_xml() const;

        // 消息处理过程
		virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

        // 系统控件消息处理
		virtual LRESULT control_message(SystemControl* ctl, UINT umsg, WPARAM wparam, LPARAM lparam);

        // 过滤窗口句柄为 SystemControl 的子控件并由其替代处理消息
        // 如果某个 SystemControl 自身创建了控件，那么此控件不受此界面库控件
        // 需要用户手动过滤，将其消息转发给对应的 SystemControl 处理
        // 比如 ListView，它的表头是一个 HeaderControl，主窗口消息过程那里也会收到
        // 算来此 HeaderContro 的消息，但此消息转发给 ListView 处理更为合适，所以
        // 有了此 filter_control 过程
        virtual SystemControl* filter_control(HWND hwnd) { return nullptr; }

        // 处理菜单消息（以字符串ID列表的形式）
		virtual LRESULT on_menu(const MenuIDs& ids) { return 0; }
        
        // 处理快捷键
        virtual LRESULT on_accel(int id) { return 0; }

        // 处理 SystemControl 的通知消息
        // 由 WM_NOTIFY 和 WM_COMMAND 合并而来
		virtual LRESULT on_notify(HWND hwnd, Control* pc, int code, NMHDR* hdr) { return 0; }

        // 包装一个异步调用添加函数
        void async_call(std::function<void()> fn) {
            __window_manager.add_async_call(fn);
        }

	private:
        // 重载掉基类的消息处理过程，用于处理界面消息
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;

        // 全局控件子类化处理过程，转发消息给对应的控件
        static LRESULT __stdcall __control_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	protected:
        bool                    _window_created;    // 标识窗口是否已经创建了
        WindowContainer*        _window;            // 窗口容器，用于容纳界面元素
        RootControl*            _root;              // 界面元素根控件
        ResourceManager         _mgr;               // 本窗口的资源管理器
        Array<const MenuItem*>  _menus;             // 本窗口的菜单集
	};

    // 全局初始化函数
    void init();

    // 执行全局消息循环
    int loop_message();
}

#endif//__taowin_h__
