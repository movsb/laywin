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

            if(tag == "control")            ctl = new control;
            else if(tag == "container")     ctl = new container;

            else if(tag == "horizontal")    ctl = new horizontal;
            else if(tag == "vertical")      ctl = new vertical;

            else if(tag == "button")        ctl = new button;
            else if(tag == "option")        ctl = new option;
            else if(tag == "check")         ctl = new check;
            else if(tag == "label")         ctl = new label;
            else if(tag == "group")         ctl = new group;
            else if(tag == "edit")          ctl = new edit;
            else if(tag == "listview")      ctl = new listview;

            else                            ctl = nullptr;

            if(!ctl) {
                assert(0);
                return;
            }

            ctl->create(p->hwnd(), c->attrs(), *mgr);

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
    {

    }

    window_creator::~window_creator() {
        delete _root;
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
                return on_menu(id, code != 0);
            }

			break;
		}
		case WM_NOTIFY:
		{
			NMHDR* hdr = reinterpret_cast<NMHDR*>(lparam);
            control* pc = (control*)::GetWindowLongPtr(hdr->hwndFrom, GWL_USERDATA);
			if(!hdr || !pc) break;
			return on_notify(hdr->hwndFrom, pc, hdr->code, hdr);
		}
        case WM_CREATE:
        {
            center();

            using namespace parser;
            PARSER_OBJECT* p = parser::parse(get_skin_xml());
            if(!p) break;

            if(p->tag == "window") {
                auto window = new window_container;
                window->_hwnd = _hwnd;
                p->dump_attr([&](const char* a, const char* v) {
                    window->set_attr(a, v);
                });

                window_meta_t metas;
                get_metas(&metas); // only styles
                rect rc {0, 0, window->_init_size.cx, window->_init_size.cy};
                ::AdjustWindowRectEx(&rc, metas.style|WS_VISIBLE, FALSE, metas.exstyle);
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
                                _mgr.add_font(name.c_str(), face.c_str(), size);
                            }
                        });
                    } else if(c->tag == "root") {
                        if(_root) delete _root; // delete fake
                        _root = new root_control;
                        _root->hwnd(_hwnd);

                        _create_children(_root, c, &_mgr);
                    }
                });

                rect rc2;
                ::GetClientRect(_hwnd, &rc2);
                _root->pos({0, 0, rc2.width(), rc2.height()});
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
            return (LRESULT)::GetStockObject(WHITE_BRUSH);
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
