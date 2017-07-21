#define NOMINMAX
#include <windows.h>
#include <tchar.h>
#include <string>
#include <algorithm>

#include "tw_util.h"
#include "tw_resmgr.h"
#include "tw_control.h"

namespace taowin{
	
	Control::Control()
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

	Control::~Control()
	{

	}

	bool Control::focus()
	{
		if (::IsWindow(_hwnd)){
			::SetFocus(_hwnd);
			return true;
		}

		return false;
	}

	Control* Control::find(HWND h)
	{
		return h == _hwnd ? this : NULL;
	}

	Control* Control::find(LPCTSTR n)
	{
		return n == _name ? this : NULL;
	}

	void Control::need_parent_update()
	{
        if (_parent){
            _parent->need_update();
        }
        else{
            need_update(); // root only
        }
	}

	void Control::set_attr(const TCHAR* name, const TCHAR* value)
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

	Size Control::estimate_size(const Size& available)
	{
		return { _width, _height };
	}

	void Control::pos(const Rect& rc)
	{
		_pos = rc;
		if(_pos.right < _pos.left) _pos.right = _pos.left;
		if(_pos.bottom < _pos.top)  _pos.bottom = _pos.top;

        _post_size = {_pos.width(), _pos.height()};

		if(!IsWindow(_hwnd))
			return;

		Rect rct = _pos;

		rct.left    += _padding.left;
		rct.top     += _padding.top;
		rct.right   -= _padding.right;
		rct.bottom  -= _padding.bottom;

        if(!is_container()) {
            ::SetWindowPos(_hwnd, 0, rct.left, rct.top, rct.width(), rct.height(), SWP_NOZORDER);
        }
	}

    void Control::create(HWND parent, std::map<string, string>& attrs, ResourceManager& mgr) {
        for(auto it = attrs.cbegin(); it != attrs.cend(); it++)
            set_attr(it->first.c_str(), it->second.c_str());
    }

    //////////////////////////////////////////////////////////////////////////
	void Container::pos(const Rect& rc)
	{
		__super::pos(rc);

		if(_items.size() == 0) return;

		Rect r = _pos;
		r.left      += _padding.left;
		r.top       += _padding.top;
		r.right     -= _padding.right;
		r.bottom    -= _padding.bottom;

		for(int i = 0; i < _items.size(); i++){
			Control* c = _items[i];
			if(!c->is_visible()) continue;
			c->pos(r);
		}
	}

	void Container::set_visible(bool visible_)
	{
		_b_visible = visible_;

		for(int i = 0; i < _items.size(); i++){
			_items[i]->set_visible_by_parent(Control::is_visible());
		}

		need_parent_update();
	}

	void Container::displayed(bool displayed_)
	{
		_b_displayed = displayed_;

		for(int i = 0; i < _items.size(); i++){
			_items[i]->set_visible_by_parent(Control::is_visible());
		}

		need_update();
	}

	Control* Container::find(LPCTSTR n)
	{
		if(_name == n) return this;

		for(int i = 0; i < _items.size(); i++){
			Control* pc = _items[i]->find(n);
			if(pc) return pc;
		}
		return NULL;
	}

	Control* Container::find(HWND h)
	{
		if(h == _hwnd) return this;

		for(int i = 0; i < _items.size(); ++i){
			Control* pc = _items[i]->find(h);
			if(pc) return pc;
		}

		return NULL;
	}

	void Horizontal::pos(const Rect& rc)
	{
		Control::pos(rc);

		if(_items.size() == 0) return;

		Rect r = _pos;
        r.left      += _padding.left;
		r.top       += _padding.top;
		r.right     -= _padding.right;
		r.bottom    -= _padding.bottom;

		Size szAvailable = {r.width(), r.height()};

		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimate = 0;

		for(int i = 0; i < _items.size(); i++){
			Control* pc = _items[i];
			if(!pc->is_visible()) continue;

			Size sz = pc->estimate_size(szAvailable);
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
			cxExpand = std::max(0, (szAvailable.cx - cxFixed) / nAdjustables);

		Size szRemaining = szAvailable;
		int iPosX = r.left;
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;

		for(int i = 0; i < _items.size(); i++){
			Control* pc = _items[i];
			if(!pc->is_visible()) continue;

			Size sz = pc->estimate_size(szRemaining);
			if(sz.cx == 0){
				iAdjustable++;
				sz.cx = cxExpand;

				if(iAdjustable == nAdjustables){
					sz.cx = std::max(0, szRemaining.cx - cxFixedRemaining);
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

			Rect rct = {iPosX, r.top, iPosX + sz.cx, r.top + sz.cy};
			pc->pos(rct);
			iPosX += sz.cx;
			cxNeeded += sz.cx;
			szRemaining.cx -= sz.cx;
		}
        _post_size = {cxNeeded + _padding.left + _padding.right, _pos.height()};//sztmp;
	}


	void Vertical::pos(const Rect& rc)
	{
		Control::pos(rc);

		if(_items.size() == 0) return;

		Rect r = _pos;
        r.left      += _padding.left;
		r.top       += _padding.top;
		r.right     -= _padding.right;
		r.bottom    -= _padding.bottom;

		Size szAvailable = {r.width(), r.height()};

		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimate = 0;

		for(int i = 0; i < _items.size(); i++){
			Control* pc = _items[i];
			if(!pc->is_visible()) continue;

			Size sz = pc->estimate_size(szAvailable);
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
			cyExpand = std::max(0, (szAvailable.cy - cyFixed) / nAdjustables);

		Size szRemaining = szAvailable;
		int iPosY = r.top;
		int iPosX = r.left;
		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;

		for(int i = 0; i < _items.size(); i++){
			Control* pc = _items[i];
			if(!pc->is_visible()) continue;

			Size sz = pc->estimate_size(szRemaining);
			if(sz.cy == 0){
				iAdjustable++;
				sz.cy = cyExpand;

				if(iAdjustable == nAdjustables){
					sz.cy = std::max(0, szRemaining.cy - cyFixedRemaining);
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

			Rect rct = {iPosX, iPosY, iPosX + sz.cx, iPosY + sz.cy};
			pc->pos(rct);

			iPosY += sz.cy;
			cyNeeded += sz.cy;
			szRemaining.cy -= sz.cy;
		}
        _post_size = {_pos.width(), cyNeeded + _padding.top + _padding.bottom};
	}

	WindowContainer::WindowContainer()
	{
		_init_size.cx = 512;
		_init_size.cy = 480;
	}

	void WindowContainer::set_attr(const TCHAR* name, const TCHAR* value)
	{
		if(_tcscmp(name, _T("title")) == 0){
            ::SetWindowText(_hwnd, value);
		}
		else if(_tcscmp(name, _T("size")) == 0){
			int w, h;
			if(_stscanf(value, _T("%d,%d"), &w, &h) == 2){
				_init_size.cx = w;
				_init_size.cy = h;
			}
		}
		else
			return __super::set_attr(name, value);
	}
}
