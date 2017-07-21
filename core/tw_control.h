#pragma once

#include <map>
#include "tw_util.h"

namespace taowin{

	class ResourceManager;
	class Container;

	class Control {
        friend class Container;
        friend class Horizontal;
        friend class Vertical;
	public:
		Control();
		virtual ~Control();

        virtual void create(HWND parent, std::map<string, string>& attrs, ResourceManager& mgr);

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

		virtual Size estimate_size(const Size& available);

        virtual void set_attr(const TCHAR* name, const TCHAR* value);

		virtual void need_update(){
            pos(_pos);
		}

		virtual void need_parent_update();

		virtual Container* parent() const{
			return reinterpret_cast<Container*>(_parent);
		}
		virtual void parent(Container* pa){
			_parent = reinterpret_cast<Control*>(pa);
		}

		virtual Control* find(LPCTSTR n);
		virtual Control* find(HWND h);

	protected:
        string          _name;      // 控件的名字，不要重复
		HWND            _hwnd;      // 所绑定的控件的句柄

		Rect            _pos;       // 在界面上的位置
        Size           _post_size; // 最终被设置的大小
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

		Control*        _parent;    // 父控件

		void*           _ud;        // 用户数据
	};

	class Container : public Control
	{
	public:
		Container()
			: _enable_update(true)
		{}

		virtual ~Container(){
			remove_all();
		}

		virtual bool is_container() const override { return true; }

		int size() const{
			return _items.size();
		}

		bool add(Control* c){
			return _items.add(c);
		}

		bool remove(Control* c){
			return _items.remove(c);
		}

		bool remove_all(){
			_items.empty();
			return true;
		}

		Control* operator[](int i){
			return _items[i];
		}

		virtual void pos(const Rect& rc) override;

		virtual void set_visible(bool visible_) override;
		virtual void displayed(bool displayed_) override;

		virtual Control* find(LPCTSTR n) override;
		virtual Control* find(HWND h) override;
        template<class T>
        T* find(LPCTSTR n) {
            return static_cast<T*>(find(n));
        }
        template<class T>
        T* find(HWND n) {
            return static_cast<T*>(find(n));
        }

	protected:
		Array<Control*> _items;
		bool _enable_update;
	};

	class Horizontal : public Container
	{
	public:
		virtual void pos(const Rect& rc) override;
	};

	class Vertical : public Container
	{
	public:
		virtual void pos(const Rect& rc) override;
	};

	class WindowContainer : public Container
	{
        friend class WindowCreator;
	public:
		WindowContainer();

	protected:
		void resmgr_(ResourceManager* mgr);
		virtual void set_attr(const TCHAR* name, const TCHAR* value) override;

    private:
		Size _init_size;
	};

    class RootControl : public Container {
    public:
        const Size& get_post_size() const {
            return _post_size;
        }
    };
}
