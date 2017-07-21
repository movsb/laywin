#include <assert.h>

#include "tw_taowin.h"
#include "tw_parser.h"

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
		return __super::__handle_message(umsg, wparam, lparam);
	}

	LRESULT WindowCreator::__handle_message(UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		switch(umsg)
		{
        case WM_SIZE:
            if(!_window_created) return 0;

            _root->pos({0, 0, LOWORD(lparam), HIWORD(lparam)});
			break;
		case WM_COMMAND:
		{
			HWND hwnd = HWND(lparam);
			int id = LOWORD(wparam);
			int code = HIWORD(wparam);

            if(hwnd) { // from control message
                SystemControl* pc = (SystemControl*)::GetWindowLongPtr(hwnd, GWL_USERDATA);
                LRESULT lr = 0;
                if(pc) {
                    NMHDR hdr;
                    hdr.hwndFrom = hwnd;
                    hdr.idFrom = id;
                    hdr.code = code;
                    if(pc->filter_notify(code, &hdr, &lr)) {
                        return lr;
                    }
                }
                else {
                    pc = filter_control(hwnd);
                    if(pc) {
                        NMHDR hdr;
                        hdr.hwndFrom = hwnd;
                        hdr.idFrom = id;
                        hdr.code = code;
                        if(pc->filter_child(hwnd, code, &hdr, &lr)) {
                            return lr;
                        }
                    }
                }

				if (pc != nullptr) {
					return on_notify(hwnd, pc, code, nullptr);
				}
            }
            else {
                if(code == 0) {
                    for(int i = 0; i < _menus.size(); ++i) {
                        MenuIds ids = _menus[i]->get_ids(id);
                        if(!ids->empty()) {
                            return on_menu(ids);
                        }
                    }
                }
                else {
                    return on_accel(id);
                }
            }

			break;
		}
		case WM_NOTIFY:
		{
			NMHDR* hdr = reinterpret_cast<NMHDR*>(lparam);
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
        case WM_CREATE:
        {
            using namespace parser;

            const TCHAR* err = nullptr;
            PARSER_OBJECT* p = parser::parse(get_skin_xml(), &err);
            if(!p) {
                if(err) {
                    msgbox(err, MB_ICONERROR, _T(""));
                }
                break;
            }

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
                    if(c->tag == _T("Resource")) {
                        c->dump_children([&](PARSER_OBJECT* c) {
                            if(c->tag == _T("Font")) {
                                string name = c->get_attr(_T("name"));
                                string face = c->get_attr(_T("face"));
                                int size = std::stoi(c->get_attr(_T("size"), _T("12")));
                                _mgr.add_font(name.c_str(), face.c_str(), size);
                            }
                        });
                    } else if(c->tag == _T("Root")) {
                        if(_root) delete _root; // delete fake
                        _root = new RootControl;
                        _root->hwnd(_hwnd);

                        _create_children(_root, c, &_mgr);
                    }
                });

                Rect rc {0, 0, _window->_init_size.cx, _window->_init_size.cy};
                ::AdjustWindowRectEx(&rc, metas.style|WS_VISIBLE, FALSE, metas.exstyle);
                rc.offset(-rc.left, -rc.top);
                ::SetWindowPos(_hwnd, nullptr, rc.left, rc.top, rc.width(), rc.height(), SWP_NOZORDER);

                if (metas.flags & WindowFlag::center)
                    center();

                Rect rc2;
                ::GetClientRect(_hwnd, &rc2);
                _root->pos({0, 0, rc2.width(), rc2.height()});
            }

            _window_created = true;

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

        ::ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
        ::ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
        ::ChangeWindowMessageFilter(0x49 /* WM_COPYGLOBALDATA */, MSGFLT_ADD);
    }

}
