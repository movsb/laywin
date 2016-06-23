#pragma once

#include <map>
#include "tw_util.h"

namespace taowin{

	class resmgr;
	class container;

	class control {
        friend class container;
        friend class horizontal;
        friend class vertical;
	public:
		control();
		virtual ~control();

        virtual void create(HWND parent, std::map<string, string>& attrs, resmgr& mgr);

        HWND hwnd() const {
            return _hwnd;
        }

        void hwnd(HWND hwnd_) {
            _hwnd = hwnd_;
        }

        const string& name() {
            return _name;
        }

        void set_enabled(bool enable = true) {
            ::EnableWindow(_hwnd, enable);
        }

		virtual bool is_container() const { return false; }

		virtual bool focus();

		virtual void pos(const Rect& rc);

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

		Rect            _pos;       // 在界面上的位置
        csize           _post_size; // 最终被设置的大小
		Rect            _padding;   // 内边距

		int             _width;     // 设置的宽度，默认为0
		int             _height;    // 设置的高度，默认为0
		int             _min_width; // 最小宽度
		int             _max_width; // 最大宽度
		int             _min_height;// 最小高度
		int             _max_height;// 最大高度


		bool            _b_visible;             // 此控件的显示属性，隐藏时也会占空间
		bool            _b_visible_by_parent;   // 父控件所设置的显示属性
		bool            _b_displayed;           // 同CSS，false时不占空间

		control*        _parent;    // 父控件

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

		virtual bool is_container() const override { return true; }

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

		virtual void pos(const Rect& rc) override;

		virtual void set_visible(bool visible_) override;
		virtual void displayed(bool displayed_) override;

		virtual control* find(LPCTSTR n) override;
		virtual control* find(HWND h) override;
        template<class T>
        T* find(LPCTSTR n) {
            return static_cast<T*>(find(n));
        }
        template<class T>
        T* find(HWND n) {
            return static_cast<T*>(find(n));
        }

	protected:
		array<control*> _items;
		bool _enable_update;
	};

	class horizontal : public container
	{
	public:
		virtual void pos(const Rect& rc) override;
	};

	class vertical : public container
	{
	public:
		virtual void pos(const Rect& rc) override;
	};

	class window_container : public container
	{
        friend class window_creator;
	public:
		window_container();

	protected:
		void resmgr_(resmgr* mgr);
		virtual void set_attr(const char* name, const char* value) override;

    private:
		csize _init_size;
	};

    class root_control : public container {
    public:
        const csize& get_post_size() const {
            return _post_size;
        }
    };
}
