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

    // �����Ĵ��ڻ���
    class WindowCreator : public Window
	{
    public:
        WindowCreator();
        virtual ~WindowCreator();

        // ���໯ĳ��ϵͳ�ؼ�
        void subclass_control(SystemControl* ctl);

        // ���һ���˵��������Զ��ַ��˵���Ϣ
        void add_menu(const MenuItem* menu) { _menus.add(menu); }

	protected:
        // ��ȡƤ�� XML �ļ�
		virtual LPCTSTR get_skin_xml() const;

        // ��Ϣ�������
		virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

        // ϵͳ�ؼ���Ϣ����
		virtual LRESULT control_message(SystemControl* ctl, UINT umsg, WPARAM wparam, LPARAM lparam);

        // ���˴��ھ��Ϊ SystemControl ���ӿؼ����������������Ϣ
        // ���ĳ�� SystemControl �������˿ؼ�����ô�˿ؼ����ܴ˽����ؼ�
        // ��Ҫ�û��ֶ����ˣ�������Ϣת������Ӧ�� SystemControl ����
        // ���� ListView�����ı�ͷ��һ�� HeaderControl����������Ϣ��������Ҳ���յ�
        // ������ HeaderContro ����Ϣ��������Ϣת���� ListView �����Ϊ���ʣ�����
        // ���˴� filter_control ����
        virtual SystemControl* filter_control(HWND hwnd) { return nullptr; }

        // ����˵���Ϣ�����ַ���ID�б����ʽ��
		virtual LRESULT on_menu(const MenuIDs& ids) { return 0; }
        
        // �����ݼ�
        virtual LRESULT on_accel(int id) { return 0; }

        // ���� SystemControl ��֪ͨ��Ϣ
        // �� WM_NOTIFY �� WM_COMMAND �ϲ�����
		virtual LRESULT on_notify(HWND hwnd, Control* pc, int code, NMHDR* hdr) { return 0; }

        // ��װһ���첽������Ӻ���
        void async_call(std::function<void()> fn) {
            __window_manager.add_async_call(fn);
        }

	private:
        // ���ص��������Ϣ������̣����ڴ��������Ϣ
		virtual LRESULT __handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;

        // ȫ�ֿؼ����໯������̣�ת����Ϣ����Ӧ�Ŀؼ�
        static LRESULT __stdcall __control_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	protected:
        bool                    _window_created;    // ��ʶ�����Ƿ��Ѿ�������
        WindowContainer*        _window;            // �����������������ɽ���Ԫ��
        RootControl*            _root;              // ����Ԫ�ظ��ؼ�
        ResourceManager         _mgr;               // �����ڵ���Դ������
        Array<const MenuItem*>  _menus;             // �����ڵĲ˵���
	};

    // ȫ�ֳ�ʼ������
    void init();

    // ִ��ȫ����Ϣѭ��
    int loop_message();
}

#endif//__taowin_h__
