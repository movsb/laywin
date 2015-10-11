#include "lw_laywin.h"
#include "lw_parser.h"

namespace laywin{

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

	void laywin::set_layout(LPCTSTR json, HWND hwnd)
	{
        /*
		_root = parser::parse(json, &_mgr);

		_mgr.hwnd(hwnd);
		_root->manager_(&_mgr);

		_root->font(-2);
		_root->visible(_root->control::visible());

		_root->init();
		*/
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

	LRESULT window_creator::handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled)
	{
		return __super::__handle_message(umsg, wparam, lparam, handled);
	}

	LRESULT window_creator::__handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled)
	{
		switch(umsg)
		{
		case WM_SIZE:
			_layout.set_size(LOWORD(lparam), HIWORD(lparam));
			break;
		case WM_CREATE:
			try{
				_layout.set_layout(get_skin_json(), _hwnd);
			}
			catch(LPCTSTR e){
				::MessageBox(_hwnd, e, NULL, MB_ICONERROR);
			}
			break;
		case WM_COMMAND:
		{
			HWND hwnd = HWND(lparam);
			int id = LOWORD(wparam);
			int code = HIWORD(wparam);

			if(hwnd == 0 && code == 0){
				if(id == 2){
					response_default_key_event(_hwnd, VK_ESCAPE);
					return 0;
				}
				else if(id == 1){
					response_default_key_event(_hwnd, VK_RETURN);
					return 0;
				}
			}

			if(hwnd){
				control* pc = _layout.root()->find(hwnd);
				//if(pc) return on_command_ctrl(hwnd, pc, code);
				return on_notify_ctrl(hwnd, pc, code, nullptr);
			}
			else{
				if(HIWORD(wparam) == 0){
					return on_menu(LOWORD(wparam));
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
			if(!lparam) break;
			NMHDR* hdr = reinterpret_cast<NMHDR*>(lparam);
			control* pc = _layout.root()->find(hdr->hwndFrom);
			if(!pc) break;
			return on_notify_ctrl(hdr->hwndFrom, pc, hdr->code, hdr);
		}
		}

		return handle_message(umsg, wparam, lparam, handled);
	}

	void window_creator::on_first_message(HWND hwnd)
	{
		__super::on_first_message(hwnd);
	}

	void window_creator::on_final_message(HWND hwnd)
	{
		return __super::on_final_message(hwnd);
	}

    static void _create_children(container* p, parser::PARSER_OBJECT* o, manager* mgr) {
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
                ::SendMessage(ctl->hwnd(), WM_SETFONT, WPARAM(mgr->default_font()), TRUE);

            p->add(ctl);

            if(ctl->is_container() && c->count_children()) {
                ctl->hwnd(p->hwnd());
                _create_children((container*)ctl, c, mgr);
            }
        });
    }

    HWND window_creator::create(HWND hParent/*=NULL*/, DWORD dwStyle/*=0*/, DWORD dwExStyle/*=0*/, HMENU hMenu /*= NULL*/) {
        using namespace parser;
        PARSER_OBJECT* p = parser::parse(get_skin_json(), nullptr);

        container* window = nullptr;

        if(p->tag == "window") {
            window = new container;
            p->dump_attr([&](const char* a, const char* v) {
                window->set_attr(a, v);
            });

            register_window_class();
            _hwnd = ::CreateWindowEx(0, get_window_class_name(), "test", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, nullptr, this);
            rect rc {100, 100, 256, 240};
            ::AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW|WS_VISIBLE, FALSE, 0);
            ::SetWindowPos(_hwnd, nullptr, rc.left, rc.top, rc.width(), rc.height(), SWP_NOZORDER);

            p->dump_children([&](PARSER_OBJECT* c) {
                if(c->tag == "res") {
                    c->dump_children([&](PARSER_OBJECT* c) {
                        if(c->tag == "font") {
                            std::string name = c->get_attr("name");
                            std::string face = c->get_attr("face");
                            int size = std::stoi(c->get_attr("size", "12"));

                            _layout._mgr.add_font(name.c_str(), face.c_str(), size, false, false, false);

                            std::string s = c->get_attr("default", "false");
                            if(!s.size() || s == "true")
                                _layout._mgr.default_font(face.c_str(), size, false, false, false);
                        }
                    });
                } else if(c->tag == "root") {
                    _layout._root = new container;
                    _layout._root->hwnd(_hwnd);

                    _create_children(_layout._root, c, &_layout._mgr);
                }
            });

            _layout->init();
            rect rc2;
            ::GetClientRect(_hwnd, &rc2);
            _layout.set_size(rc2.width(), rc2.height());
        }
        return _hwnd;
    }
}
