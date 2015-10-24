#include "lw_laywin.h"
#include "lw_parser.h"

namespace laywin{
    // TODO this is temporary
    // lambda cannot be called by itself, so we separated it here
    static void _create_children(container* p, parser::PARSER_OBJECT* o, resmgr* mgr) {
        o->dump_children([&](parser::PARSER_OBJECT* c) {
            control* ctl = nullptr;
            auto& tag = c->tag;

            if(tag == "control")            ctl = new control;
            else if(tag == "container")     ctl = new container;

            else if(tag == "horizontal")    ctl = new horizontal;
            else if(tag == "vertical")      ctl = new vertical;

            else if(tag == "button")        ctl = new button;
            else if(tag == "edit")          ctl = new edit;

            else                            ctl = nullptr;

            if(!ctl) return;

            std::string s;
            s = c->get_attr("exstyle");
            if(s.size()) {
                ctl->set_attr("exstyle", s.c_str());
            }

            ctl->create(p->hwnd());

            c->dump_attr([&](const char* n, const char* v) {
                ctl->set_attr(n, v);
            });

            if(!c->has_attr("font"))
                ::SendMessage(ctl->hwnd(), WM_SETFONT, WPARAM(mgr->get_font("default")), TRUE);

            p->add(ctl);

            if(ctl->is_container() && c->count_children()) {
                ctl->hwnd(p->hwnd());
                _create_children((container*)ctl, c, mgr);
            }
        });
    }

	laywin::laywin()
		: _root(NULL)
	{

	}

	laywin::~laywin()
	{
		if(_root){
			delete _root;
			_root = NULL;
		}
	}

	void laywin::set_size(int cx, int cy)
	{
        if(_root)
            _root->pos({0, 0, cx, cy});
	}


	LPCTSTR window_creator::get_skin_json() const
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
			_layout.set_size(LOWORD(lparam), HIWORD(lparam));
			break;
		case WM_COMMAND:
		{
			HWND hwnd = HWND(lparam);
			int id = LOWORD(wparam);
			int code = HIWORD(wparam);

            if(hwnd) { // from control message
                control* pc = _layout.root()->find(hwnd);   // TODO
                if(pc) {
                    return on_notify(hwnd, pc, code, nullptr);
                }
            }
            else {
                return on_menu(id, code != 0);
            }

			break;
		}
		case WM_NOTIFY:
		{
			NMHDR* hdr = reinterpret_cast<NMHDR*>(lparam);
			control* pc = _layout.root()->find(hdr->hwndFrom); // TODO set window extra to pc/name
			if(!hdr || !pc) break;
			return on_notify(hdr->hwndFrom, pc, hdr->code, hdr);
		}
        case WM_CREATE:
        {
            using namespace parser;
            PARSER_OBJECT* p = parser::parse(get_skin_json(), nullptr);
            if(!p) break;

            if(p->tag == "window") {
                auto window = new window_container;
                window->_hwnd = _hwnd;
                p->dump_attr([&](const char* a, const char* v) {
                    window->set_attr(a, v);
                });

                rect rc {0, 0, window->_init_size.cx, window->_init_size.cy};
                ::AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW|WS_VISIBLE, FALSE, 0);
                rc.offset(-rc.left, -rc.top);
                ::SetWindowPos(_hwnd, nullptr, rc.left, rc.top, rc.width(), rc.height(), SWP_NOZORDER);
                center();

                p->dump_children([&](PARSER_OBJECT* c) {
                    if(c->tag == "res") {
                        c->dump_children([&](PARSER_OBJECT* c) {
                            if(c->tag == "font") {
                                std::string name = c->get_attr("name");
                                std::string face = c->get_attr("face");
                                int size = std::stoi(c->get_attr("size", "12"));
                                _layout._mgr.add_font(name.c_str(), face.c_str(), size);
                            }
                        });
                    } else if(c->tag == "root") {
                        _layout._root = new container;
                        _layout._root->hwnd(_hwnd);

                        _create_children(_layout._root, c, &_layout._mgr);
                    }
                });

                _layout.root()->init();
                rect rc2;
                ::GetClientRect(_hwnd, &rc2);
                _layout.set_size(rc2.width(), rc2.height());
            }
            break;
        }
		}

		return handle_message(umsg, wparam, lparam);
	}

    int loop_message() {
        return __window_manager.loop_message();
    }

    void init() {
        register_window_classes();
    }

}
