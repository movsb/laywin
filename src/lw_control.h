#pragma once

#include "lw_util.h"

namespace laywin{

	class resmgr;
	class container;

	class control {
        friend class container;
        friend class horizontal;
        friend class vertical;
	public:
		control();
		virtual ~control();

        virtual void create(HWND parent) {
            
        }

        HWND hwnd() const {
            return _hwnd;
        }

        void hwnd(HWND hwnd_) {
            _hwnd = hwnd_;
        }

        virtual void set_resmgr(resmgr* mgr) {
            _mgr = mgr;
        }

		virtual bool is_container() const { return false; }

		virtual LPCTSTR get_class() const { return get_class_static(); }
		static LPCTSTR get_class_static() { return _T("control"); }

		virtual void init();
		virtual bool focus();

		virtual void pos(const rect& rc);

		virtual int		width() const { return _pos.right - _pos.left; }
		virtual int		height() const { return _pos.bottom - _pos.top; }

		bool    is_visible() const {
			return _b_visible && _b_visible_by_parent;
		}

		virtual void set_visible(bool visible_){
			_b_visible = visible_;
			displayed(displayed());
			need_parent_update();
		}

		virtual void set_visible_by_parent(bool visible_){
			_b_visible_by_parent = visible_;
			displayed(displayed());
		}

		virtual bool displayed() const {
			return _b_displayed;
		}

		virtual void displayed(bool displayed_){
			_b_displayed = displayed_;
			if (::IsWindow(_hwnd)){
				::ShowWindow(_hwnd, is_visible() ? SW_SHOW : SW_HIDE);
			}
		}

		virtual csize estimate_size(const csize& available);

        virtual void set_attr(const char* name, const char* value);

		virtual void need_update(){
            pos(_pos);
		}

		virtual void need_parent_update();

		virtual container* parent() const{
			return reinterpret_cast<container*>(_parent);
		}
		virtual void parent(container* pa){
			_parent = reinterpret_cast<control*>(pa);
		}

		virtual control* find(LPCTSTR n);
		virtual control* find(HWND h);

	protected:
        string          _name;      // 控件的名字，不要重复
		HWND            _hwnd;      // 所绑定的控件的句柄

		rect            _pos;       // 在界面上的位置
		rect            _padding;   // 内边距

		int             _width;     // 设置的宽度，默认为0
		int             _height;    // 设置的高度，默认为0
		int             _min_width; // 最小宽度
		int             _max_width; // 最大宽度
		int             _min_height;// 最小高度
		int             _max_height;// 最大高度


		bool            _b_visible;             // 此控件的显示属性，隐藏时也会占空间
		bool            _b_visible_by_parent;   // 父控件所设置的显示属性
		bool            _b_displayed;           // 同CSS，false时不占空间

		resmgr*         _mgr;       // 资源管理器
		control*        _parent;    // 父控件

		string          _font;      // 控件字体

		void*           _ud;        // 用户数据
	};

	class container : public control
	{
	public:
		container()
			: _enable_update(true)
		{}

		virtual ~container(){
			remove_all();
		}

        virtual void set_resmgr(resmgr* mgr) override;
		virtual bool is_container() const override { return true; }

		static LPCTSTR get_class_static() { return _T("container"); }
		virtual LPCTSTR get_class() const override { return get_class_static(); }

		virtual void init() override{
			__super::init();

			for(int c = size(), i = 0; i < c; ++i){
				_items[i]->init();
			}
		}

		int size() const{
			return _items.size();
		}

		bool add(control* c){
			return _items.add(c);
		}

		bool remove(control* c){
			return _items.remove(c);
		}

		bool remove_all(){
			_items.empty();
			return true;
		}

		control* operator[](int i){
			return _items[i];
		}

		virtual void pos(const rect& rc) override;

		virtual void set_visible(bool visible_) override;
		virtual void displayed(bool displayed_) override;

		virtual control* find(LPCTSTR n) override;
		virtual control* find(HWND h) override;

	protected:
		array<control*> _items;
		bool _enable_update;
	};

	class horizontal : public container
	{
	public:
		virtual LPCTSTR get_class() const override { return get_class_static(); }
		static LPCTSTR get_class_static() { return _T("horizontal"); }

		virtual void pos(const rect& rc) override;
	};

	class vertical : public container
	{
	public:
		virtual LPCTSTR get_class() const override { return get_class_static(); }
		static LPCTSTR get_class_static() { return _T("vertical"); }

		virtual void pos(const rect& rc) override;
	};

	class window_container : public container
	{
        friend class window_creator;
	public:
		window_container();

	protected:
		virtual void init() override;
		void resmgr_(resmgr* mgr);
		virtual void set_attr(const char* name, const char* value) override;

    private:
		csize _init_size;
	};
}
