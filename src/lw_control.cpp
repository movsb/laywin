#include <windows.h>
#include <tchar.h>
#include <string>

#include "lw_util.h"
#include "lw_resmgr.h"
#include "lw_control.h"

namespace laywin{
	
	control::control()
		: _hwnd(nullptr)
		, _width(0)
		, _height(0)
		, _min_width(0)
		, _min_height(0)
		, _max_width(9999)
		, _max_height(9999)
		, _b_visible(true)
		, _b_displayed(true)
		, _b_visible_by_parent(true)
		, _parent(nullptr)
		, _ud(nullptr)
	{

	}

	control::~control()
	{

	}

	void control::init()
	{
	}

	bool control::focus()
	{
		if (::IsWindow(_hwnd)){
			::SetFocus(_hwnd);
			return true;
		}

		return false;
	}

	control* control::find(HWND h)
	{
		return h == _hwnd ? this : NULL;
	}

	control* control::find(LPCTSTR n)
	{
		return n == _name ? this : NULL;
	}

	void control::need_parent_update()
	{
        if (_parent){
            _parent->need_update();
        }
        else{
            need_update(); // root only
        }
	}

	void control::set_attr(const char* name, const char* value)
	{
		if(_tcscmp(name, _T("name")) == 0){
			_name = value;
		}
		else if(_tcscmp(name, _T("padding")) == 0){
			int l, t, r, b;
			if(_stscanf(value, _T("%d,%d,%d,%d"), &l, &t, &r, &b) == 4){
				_padding = {l, t, r, b};
			}
		}
		else if(_tcscmp(name, _T("width")) == 0){
			_width = _tstoi(value);
		}
		else if(_tcscmp(name, _T("height")) == 0){
			_height = _tstoi(value);
		}
		else if(_tcscmp(name, _T("minwidth")) == 0){
			_min_width = _tstoi(value);
		}
		else if(_tcscmp(name, _T("minheight")) == 0){
			_min_height = _tstoi(value);
		}
        else if(_tcscmp(name, _T("maxwidth")) == 0) {
            _max_width = _tstoi(value);
        }
        else if(_tcscmp(name, _T("maxheight")) == 0) {
            _max_height = _tstoi(value);
        }
        else if(_tcscmp(name, _T("size")) == 0) {
            int width, height;
            if(_stscanf(value, _T("%d,%d"), &width, &height) == 2) {
                _width = width;
                _height = height;
            }
        }
		else{
			::MessageBox(NULL, value, name, MB_ICONERROR);
		}
	}

	csize control::estimate_size(const csize& available)
	{
		return { _width, _height };
	}

	void control::pos(const rect& rc)
	{
		_pos = rc;
		if(_pos.right < _pos.left) _pos.right = _pos.left;
		if(_pos.bottom < _pos.top)  _pos.bottom = _pos.top;

		csize tmpsz = {_pos.width(), _pos.height()};
//		post_size(tmpsz);

		if(!IsWindow(_hwnd))
			return;

		rect rct = _pos;

		rct.left    += _padding.left;
		rct.top     += _padding.top;
		rct.right   -= _padding.right;
		rct.bottom  -= _padding.bottom;

        if(!is_container())
		    ::SetWindowPos(_hwnd, 0, rct.left, rct.top, rct.width(), rct.height(), SWP_NOZORDER);
	}

    void control::create(HWND parent, std::map<string, string>& attrs, resmgr& mgr) {
        decltype(attrs.begin()) it;

        for(auto it = attrs.cbegin(); it != attrs.cend(); it++)
            set_attr(it->first.c_str(), it->second.c_str());
    }

    //////////////////////////////////////////////////////////////////////////
	void container::pos(const rect& rc)
	{
		__super::pos(rc);

		if(_items.size() == 0) return;

		rect r = _pos;
		r.left      += _padding.left;
		r.top       += _padding.top;
		r.right     -= _padding.right;
		r.bottom    -= _padding.bottom;

		for(int i = 0; i < _items.size(); i++){
			control* c = _items[i];
			if(!c->is_visible()) continue;
			c->pos(r);
		}
	}

	void container::set_visible(bool visible_)
	{
		_b_visible = visible_;

		for(int i = 0; i < _items.size(); i++){
			_items[i]->set_visible_by_parent(control::is_visible());
		}

		need_parent_update();
	}

	void container::displayed(bool displayed_)
	{
		_b_displayed = displayed_;

		for(int i = 0; i < _items.size(); i++){
			_items[i]->set_visible_by_parent(control::is_visible());
		}

		need_update();
	}

	control* container::find(LPCTSTR n)
	{
		if(_name == n) return this;

		for(int i = 0; i < _items.size(); i++){
			control* pc = _items[i]->find(n);
			if(pc) return pc;
		}
		return NULL;
	}

	control* container::find(HWND h)
	{
		if(h == _hwnd) return this;

		for(int i = 0; i < _items.size(); ++i){
			control* pc = _items[i]->find(h);
			if(pc) return pc;
		}

		return NULL;
	}

	void horizontal::pos(const rect& rc)
	{
		control::pos(rc);

		if(_items.size() == 0) return;

		rect r = _pos;
        r.left      += _padding.left;
		r.top       += _padding.top;
		r.right     -= _padding.right;
		r.bottom    -= _padding.bottom;

		csize szAvailable = {r.width(), r.height()};

		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimate = 0;

		for(int i = 0; i < _items.size(); i++){
			control* pc = _items[i];
			if(!pc->is_visible()) continue;

			csize sz = pc->estimate_size(szAvailable);
			if(sz.cx == 0){
				nAdjustables++;
			}
			else{
				if(sz.cx < pc->_min_width) sz.cx = pc->_min_width;
				if(sz.cx > pc->_max_width) sz.cx = pc->_max_width;
			}

			cxFixed += sz.cx;
			nEstimate++;
		}

		int cxExpand = 0;
		int cxNeeded = 0;

		if(nAdjustables > 0)
			cxExpand = max(0, (szAvailable.cx - cxFixed) / nAdjustables);

		csize szRemaining = szAvailable;
		int iPosX = r.left;
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;

		for(int i = 0; i < _items.size(); i++){
			control* pc = _items[i];
			if(!pc->is_visible()) continue;

			csize sz = pc->estimate_size(szRemaining);
			if(sz.cx == 0){
				iAdjustable++;
				sz.cx = cxExpand;

				if(iAdjustable == nAdjustables){
					sz.cx = max(0, szRemaining.cx - cxFixedRemaining);
				}

				if(sz.cx < pc->_min_width) sz.cx = pc->_min_width;
				if(sz.cx > pc->_max_width) sz.cx = pc->_max_width;
			}
			else{
				if(sz.cx < pc->_min_width) sz.cx = pc->_min_width;
				if(sz.cx > pc->_max_width) sz.cx = pc->_max_width;

				cxFixedRemaining -= sz.cx;
			}

			sz.cy = pc->_height;
			if(sz.cy == 0) sz.cy = r.height();
			if(sz.cy < 0) sz.cy = 0;
			if(sz.cy < pc->_min_height) sz.cy = pc->_min_height;
			if(sz.cy > pc->_max_height) sz.cy = pc->_max_height;

			rect rct = {iPosX, r.top, iPosX + sz.cx, r.top + sz.cy};
			pc->pos(rct);
			iPosX += sz.cx;
			cxNeeded += sz.cx;
			szRemaining.cx -= sz.cx;
		}

		csize sztmp = {cxNeeded, _pos.height()};
		sztmp.cx += _pos.left + _pos.right;
		//post_size(sztmp);
	}


	void vertical::pos(const rect& rc)
	{
		control::pos(rc);

		if(_items.size() == 0) return;

		rect r = _pos;
        r.left      += _padding.left;
		r.top       += _padding.top;
		r.right     -= _padding.right;
		r.bottom    -= _padding.bottom;

		csize szAvailable = {r.width(), r.height()};

		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimate = 0;

		for(int i = 0; i < _items.size(); i++){
			control* pc = _items[i];
			if(!pc->is_visible()) continue;

			csize sz = pc->estimate_size(szAvailable);
			if(sz.cy == 0){
				nAdjustables++;
			}
			else{
				if(sz.cy < pc->_min_height) sz.cy = pc->_min_height;
				if(sz.cy > pc->_max_height) sz.cy = pc->_max_height;
			}

			cyFixed += sz.cy;
			nEstimate++;
		}

		int cyExpand = 0;
		int cyNeeded = 0;

		if(nAdjustables > 0)
			cyExpand = max(0, (szAvailable.cy - cyFixed) / nAdjustables);

		csize szRemaining = szAvailable;
		int iPosY = r.top;
		int iPosX = r.left;
		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;

		for(int i = 0; i < _items.size(); i++){
			control* pc = _items[i];
			if(!pc->is_visible()) continue;

			csize sz = pc->estimate_size(szRemaining);
			if(sz.cy == 0){
				iAdjustable++;
				sz.cy = cyExpand;

				if(iAdjustable == nAdjustables){
					sz.cy = max(0, szRemaining.cy - cyFixedRemaining);
				}

				if(sz.cy < pc->_min_height) sz.cy = pc->_min_height;
				if(sz.cy > pc->_max_height) sz.cy = pc->_max_height;
			}
			else{
				if(sz.cy < pc->_min_height) sz.cy = pc->_min_height;
				if(sz.cy > pc->_max_height) sz.cy = pc->_max_height;

				cyFixedRemaining -= sz.cy;
			}

			sz.cx = pc->_width;
			if(sz.cx == 0) sz.cx = szAvailable.cx;
			if(sz.cx < 0) sz.cx = 0;
			if(sz.cx < pc->_min_height) sz.cx = pc->_min_width;
			if(sz.cx > pc->_max_height) sz.cx = pc->_max_width;

			rect rct = {iPosX, iPosY, iPosX + sz.cx, iPosY + sz.cy};
			pc->pos(rct);

			iPosY += sz.cy;
			cyNeeded += sz.cy;
			szRemaining.cy -= sz.cy;
		}

		csize sztmp = {_pos.width(), cyNeeded};
		sztmp.cy += _pos.top + _pos.bottom;
		//post_size(sztmp);
	}

	window_container::window_container()
	{
		_init_size.cx = 512;
		_init_size.cy = 480;
	}

	void window_container::set_attr(const char* name, const char* value)
	{
		if(_tcscmp(name, _T("title")) == 0){
            ::SetWindowText(_hwnd, value);
		}
		else if(_tcscmp(name, _T("size")) == 0){
			int w, h;
			if(_stscanf(value, "%d,%d", &w, &h) == 2){
				_init_size.cx = w;
				_init_size.cy = h;
			}
		}
		else
			return __super::set_attr(name, value);
	}

	void window_container::init()
	{
		__super::init();
		::SetWindowPos(_hwnd, NULL, 0, 0,
			_init_size.cx, _init_size.cy,
			SWP_NOMOVE | SWP_NOZORDER);
	}
}
