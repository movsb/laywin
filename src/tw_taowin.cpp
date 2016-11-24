#include <assert.h>

#include "tw_taowin.h"
#include "tw_parser.h"

namespace taowin{
    // TODO this is temporary
    // lambda cannot be called by itself, so we separated it here
    static void _create_children(container* p, parser::PARSER_OBJECT* o, resmgr* mgr) {
        o->dump_children([&](parser::PARSER_OBJECT* c) {
            control* ctl = nullptr;
            auto& tag = c->tag;

            if(tag == _T("control"))            ctl = new control;
            else if(tag == _T("container"))     ctl = new container;

            else if(tag == _T("horizontal"))    ctl = new horizontal;
            else if(tag == _T("vertical"))      ctl = new vertical;

            else if(tag == _T("button"))        ctl = new button;
            else if(tag == _T("option"))        ctl = new option;
            else if(tag == _T("check"))         ctl = new check;
            else if(tag == _T("label"))         ctl = new label;
            else if(tag == _T("group"))         ctl = new group;
            else if(tag == _T("edit"))          ctl = new edit;
            else if(tag == _T("listview"))      ctl = new listview;
            else if(tag == _T("combobox"))      ctl = new combobox;
            else if(tag == _T("tabctrl"))       ctl = new tabctrl;

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
                _create_children((container*)ctl, c, mgr);
            }
        });
    }

    window_creator::window_creator()
        : _root(new root_control)  // fake, keep _root valid always
        , _window(nullptr)
        , _window_created(false)
    {

    }

    window_creator::~window_creator() {
        delete _root;
    }

    void window_creator::subclass_control(syscontrol* ctl)
    {
        assert(::IsWindow(ctl->hwnd()));
        ctl->_old_wnd_proc = (WNDPROC)::SetWindowLongPtr(ctl->hwnd(), GWL_WNDPROC, (LONG)__control_procedure);
    }

    LPCTSTR window_creator::get_skin_xml() const
	{
		return _T("");
	}

	LRESULT window_creator::handle_message(UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		return __super::__handle_message(umsg, wparam, lparam);
	}

	LRESULT window_creator::__handle_message(UINT umsg, WPARAM wparam, LPARAM lparam)
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
                control* pc = (control*)::GetWindowLongPtr(hwnd, GWL_USERDATA);
                if(pc) {
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
            control* pc = (control*)::GetWindowLongPtr(hdr->hwndFrom, GWL_USERDATA);
			if(!hdr) break;
			return on_notify(hdr->hwndFrom, pc, hdr->code, hdr);
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

            if(p->tag == _T("window")) {
                auto window = new window_container;
                window->_hwnd = _hwnd;
                p->dump_attr([&](const TCHAR* a, const TCHAR* v) {
                    window->set_attr(a, v);
                });

                _window = window;
                _mgr._owner = this;

                WindowMeta metas;
                get_metas(&metas);

                p->dump_children([&](PARSER_OBJECT* c) {
                    if(c->tag == _T("res")) {
                        c->dump_children([&](PARSER_OBJECT* c) {
                            if(c->tag == _T("font")) {
                                string name = c->get_attr(_T("name"));
                                string face = c->get_attr(_T("face"));
                                int size = std::stoi(c->get_attr(_T("size"), _T("12")));
                                _mgr.add_font(name.c_str(), face.c_str(), size);
                            }
                        });
                    } else if(c->tag == _T("root")) {
                        if(_root) delete _root; // delete fake
                        _root = new root_control;
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

    LRESULT window_creator::control_message(syscontrol* ctl, UINT umsg, WPARAM wparam, LPARAM lparam)
    {
        return ::CallWindowProc(ctl->_old_wnd_proc, ctl->_hwnd, umsg, wparam, lparam);
    }

    LRESULT window_creator::__control_procedure(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
    {
        syscontrol* ctl = (syscontrol*)::GetWindowLongPtr(hwnd, GWL_USERDATA);
        return ctl->_owner->control_message(ctl, umsg, wparam, lparam);
    }

    int loop_message() {
        return __window_manager.loop_message();
    }

    void init() {
        register_window_classes();

        ::ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
        ::ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
        ::ChangeWindowMessageFilter(0x49 /* WM_COPYGLOBALDATA */, MSGFLT_ADD);
    }

}
