#include <assert.h>

#include "tw_taowin.h"
#include "tw_parser.h"

#ifdef TAOLOG_ENABLED
// {C9268865-FA3A-4F23-96FF-865D548C20B3}
static const GUID providerGuid =
{ 0xC9268865, 0xFA3A, 0x4F23, { 0x96, 0xFF, 0x86, 0x5D, 0x54, 0x8C, 0x20, 0xB3 } };
TaoLogger g_taoLogger(providerGuid);
#endif


namespace taowin{
    // TODO this is temporary
    // lambda cannot be called by itself, so we separated it here
    static void _create_children(Container* p, parser::PARSER_OBJECT* o, ResourceManager* mgr) {
        o->dump_children([&](parser::PARSER_OBJECT* c) {
            Control* ctl = nullptr;
            auto& tag = c->tag;

            if(tag == _T("Control"))            ctl = new Control;
            else if(tag == _T("Container"))     ctl = new Container;

            else if(tag == _T("Horizontal"))    ctl = new Horizontal;
            else if(tag == _T("Vertical"))      ctl = new Vertical;

            else if(tag == _T("Button"))        ctl = new Button;
            else if(tag == _T("RadioButton"))   ctl = new RadioButton;
            else if(tag == _T("CheckBox"))      ctl = new CheckBox;
            else if(tag == _T("Label"))         ctl = new Label;
            else if(tag == _T("Group"))         ctl = new Group;
            else if(tag == _T("TextBox"))       ctl = new TextBox;
            else if(tag == _T("ListView"))      ctl = new ListView;
            else if(tag == _T("ComboBox"))      ctl = new ComboBox;
            else if(tag == _T("TabCtrl"))       ctl = new TabCtrl;
            else if(tag == _T("Progress"))      ctl = new Progress;

            else if(tag == _T("WebView"))       ctl = new WebView;

            else                                ctl = nullptr;

            if(!ctl) {
                assert(0);
                return;
            }

            ctl->create(p->hwnd(), c->attrs(), *mgr);
            ctl->parent(p);

            // set this
            if(ctl->hwnd())
                ::SetWindowLongPtr(ctl->hwnd(), GWL_USERDATA, LONG(ctl));

            p->add(ctl);

            if(ctl->is_container() && c->count_children()) {
                ctl->hwnd(p->hwnd());
                _create_children((Container*)ctl, c, mgr);
            }
        });
    }

    WindowCreator::WindowCreator()
        : _root(new RootControl)  // fake, keep _root valid always
        , _window(nullptr)
        , _window_created(false)
    {

    }

    WindowCreator::~WindowCreator() {
        delete _root;
    }

    void WindowCreator::subclass_control(SystemControl* ctl)
    {
        assert(::IsWindow(ctl->hwnd()));
        ctl->_old_wnd_proc = (WNDPROC)::SetWindowLongPtr(ctl->hwnd(), GWL_WNDPROC, (LONG)__control_procedure);
    }

    LPCTSTR WindowCreator::get_skin_xml() const
	{
		return _T("");
	}

	LRESULT WindowCreator::handle_message(UINT umsg, WPARAM wparam, LPARAM lparam)
	{
        // 重载版本的 __handle_message 处理了界面消息
        // 这个版本的 handle_message 交给用户去重载
		return __super::__handle_message(umsg, wparam, lparam);
	}

	LRESULT WindowCreator::__handle_message(UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		switch(umsg)
		{
        // 窗口大小改变时，重新调整布局
        case WM_SIZE:
            if(!_window_created) return 0;

            _root->pos({0, 0, LOWORD(lparam), HIWORD(lparam)});
			break;

        // 分发控件消息、菜单消息、快捷键消息
		case WM_COMMAND:
		{
            /*
            +-----------------------------------------------------+
            | 消息来源 | wParam（高字）| wParam（低字）| lParam   |
            +----------+---------------+---------------+----------+
            |   菜单   |  0            |  菜单ID       | 0        |
            +----------+---------------+---------------+----------+
            |  快捷键  |  1            |  快捷键ID     | 0        |
            +----------+---------------+---------------+----------+
            |  控件    |  通知码       |  控件ID       | 控件句柄 |
            +----------+---------------+---------------+----------+
            */

			HWND hwnd = HWND(lparam);   // 窗口句柄（如果是控件消息的话）
			int id = LOWORD(wparam);    // ID
			int code = HIWORD(wparam);  // 通知码

            // 这是来自控件的消息
            if(hwnd) { // from control message
                // 如果这个控件是此界面库创建的话，那么应该有一个指针
                SystemControl* pc = (SystemControl*)::GetWindowLongPtr(hwnd, GWL_USERDATA);
                LRESULT lr = 0;
                // 的确是本界面库创建的
                if(pc) {
                    // 也包装成通知的形式下发出去
                    NMHDR hdr;
                    hdr.hwndFrom = hwnd;
                    hdr.idFrom = id;
                    hdr.code = code;

                    // SystemControl 是否需要自身处理掉通知码？
                    // 比如按钮如果有 OnClick 方法的话，可以在此
                    // 处理掉 BN_CLICK 消息
                    // 处理掉的消息就不再下发了
                    if(pc->filter_notify(code, &hdr, &lr)) {
                        return lr;
                    }
                }
                // 如果这个控件不是由界面库主动创建的
                else {
                    // 它是否来自一个由 SystemControl 控件的子控件？
                    // 过滤看看，是否能拿到
                    pc = filter_control(hwnd);
                    if(pc) {
                        NMHDR hdr;
                        hdr.hwndFrom = hwnd;
                        hdr.idFrom = id;
                        hdr.code = code;

                        // 交给 SystemControl 处理其自身的子控件的消息
                        if(pc->filter_child(hwnd, code, &hdr, &lr)) {
                            return lr;
                        }
                    }
                }

                // 说明是一个未知的、未被处理的 SystemControl 消息
                // 交给用户新版去处理
				if (pc != nullptr) {
					return on_notify(hwnd, pc, code, nullptr);
				}
            }
            else {
                // 嗯，这是菜单
                if(code == 0) {
                    for(int i = 0; i < _menus.size(); ++i) {
                        MenuIDs ids = _menus[i]->get_ids(id);
                        if(!ids->empty()) {
                            return on_menu(ids);
                        }
                    }
                }
                // 嗯，这是快捷键
                else {
                    return on_accel(id);
                }
            }

			break;
		}
        // 分发控件通知消息
		case WM_NOTIFY:
		{
			NMHDR* hdr = reinterpret_cast<NMHDR*>(lparam);
            // 这个是否有可能叫？不确定
            if(!hdr) break;

            SystemControl* pc = (SystemControl*)::GetWindowLongPtr(hdr->hwndFrom, GWL_USERDATA);
            LRESULT lr = 0;

            if(pc) {
                if(pc->filter_notify(hdr->code, hdr, &lr)) {
                    return lr;
                }
            }
            else {
                pc = filter_control(hdr->hwndFrom);
                if(pc) {
                    if(pc->filter_child(hdr->hwndFrom, hdr->code, hdr, &lr)) {
                        return lr;
                    }
                }
            }

			if (pc != nullptr) {
				return on_notify(hdr->hwndFrom, pc, hdr->code, hdr);
			}

			break;
		}
        // 窗口创建消息
        case WM_CREATE:
        {
            using namespace parser;

            // 解析 XML 界面元素
            const TCHAR* err = nullptr;
            PARSER_OBJECT* p = parser::parse(get_skin_xml(), &err);
            if(!p) {
                if(err) {
                    msgbox(err, MB_ICONERROR, _T(""));
                }
                break;
            }

            // 窗口对象
            if(p->tag == _T("Window")) {
                auto window = new WindowContainer;
                window->_hwnd = _hwnd;
                p->dump_attr([&](const TCHAR* a, const TCHAR* v) {
                    window->set_attr(a, v);
                });

                _window = window;
                _mgr._owner = this;

                WindowMeta metas;
                get_metas(&metas);

                p->dump_children([&](PARSER_OBJECT* c) {
                    // 窗口资源
                    if(c->tag == _T("Resource")) {
                        c->dump_children([&](PARSER_OBJECT* c) {
                            if(c->tag == _T("Font")) {
                                string name = c->get_attr(_T("name"));
                                string face = c->get_attr(_T("face"));
                                int size = std::stoi(c->get_attr(_T("size"), _T("12")));
                                _mgr.add_font(name.c_str(), face.c_str(), size);
                            }
                        });
                    }
                    // 窗口根节点
                    else if(c->tag == _T("Root")) {
                        if(_root) delete _root; // delete fake
                        _root = new RootControl;
                        _root->hwnd(_hwnd);

                        _create_children(_root, c, &_mgr);
                    }
                });

                // 自动调整窗口初始化大小
                Rect rc {0, 0, _window->_init_size.cx, _window->_init_size.cy};
                ::AdjustWindowRectEx(&rc, metas.style|WS_VISIBLE, FALSE, metas.exstyle);
                rc.offset(-rc.left, -rc.top);
                ::SetWindowPos(_hwnd, nullptr, rc.left, rc.top, rc.width(), rc.height(), SWP_NOZORDER);

                // 如果需要自动居中
                if (metas.flags & WindowFlag::center)
                    center();

                // 初始化布局
                Rect rc2;
                ::GetClientRect(_hwnd, &rc2);
                _root->pos({0, 0, rc2.width(), rc2.height()});
            }

            _window_created = true;

            break;
        }
        case WM_CTLCOLOREDIT:
        {
            auto hDC = (HDC)wparam;
            auto hWnd = (HWND)lparam;

            SystemControl* pc = (SystemControl*)::GetWindowLongPtr(hWnd, GWL_USERDATA);
            if (pc != nullptr) {
                TextBox* tb = static_cast<TextBox*>(pc);
                return (LRESULT)tb->handle_ctlcoloredit(hDC);
            }

            break;
        }
        case WM_CTLCOLORSTATIC:
        {
            auto hDC = (HDC)wparam;
            auto hWnd = (HWND)lparam;

            SystemControl* pc = (SystemControl*)::GetWindowLongPtr(hWnd, GWL_USERDATA);
            if (pc != nullptr) {
                if (TextBox* tb = dynamic_cast<TextBox*>(pc)) {
                    return (LRESULT)tb->handle_ctlcoloredit(hDC);
                }
            }

            break;
        }
		}

		return handle_message(umsg, wparam, lparam);
	}

    LRESULT WindowCreator::control_message(SystemControl* ctl, UINT umsg, WPARAM wparam, LPARAM lparam)
    {
        return ::CallWindowProc(ctl->_old_wnd_proc, ctl->_hwnd, umsg, wparam, lparam);
    }

    LRESULT WindowCreator::__control_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
    {
        SystemControl* ctl = (SystemControl*)::GetWindowLongPtr(hwnd, GWL_USERDATA);
        return ctl->_owner->control_message(ctl, umsg, wparam, lparam);
    }

    int loop_message() {
        return __window_manager.loop_message();
    }

    void init() {
        ::CoInitialize(nullptr);
        register_window_classes();

        // Win7 及以后的一些权限较高的消息
        // 可以通过这种方式绕过去
        ::ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
        ::ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
        ::ChangeWindowMessageFilter(0x49 /* WM_COPYGLOBALDATA */, MSGFLT_ADD);
    }

}
