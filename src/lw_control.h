#ifndef __laywin_control_h__
#define __laywin_control_h__

#include "lw_util.h"

namespace laywin{

	class manager;
	class container;

	class control {
	public:
		control();
		virtual ~control();

        virtual void create(HWND parent) {
            
        }

		virtual bool is_container() const { return false; }

		virtual LPCTSTR get_class() const { return get_class_static(); }
		static LPCTSTR get_class_static() { return _T("control"); }

		virtual void init();
		virtual bool focus();

		virtual const rect& pos() const {
			return _pos;
		}
		virtual void pos(const rect& rc);

		virtual const csize& post_size() const { return _post_size; }
		virtual void post_size(const csize& sz) { _post_size = sz; }

		virtual int		width() const { return _pos.right - _pos.left; }
		virtual int		height() const { return _pos.bottom - _pos.top; }

		virtual void	fixed_width(int cx) {
			_width = cx;
		}
		virtual int		fixed_width() const {
			return _width;
		}
		virtual void	fixed_height(int cy) {
			_height = cy;
		}
		virtual int		fixed_height() {
			return _height;
		}
		
		virtual int		min_width() const { return _min_width; }
		virtual void	min_width(int cx) {}
		virtual int		max_width() const { return _max_width; }
		virtual void	max_width(int cx) {}

		virtual int		min_height() const { return _min_height; }
		virtual void	min_height(int cy) {}
		virtual int		max_height() const { return _max_height; }
		virtual void	max_height(int cy) {}

		virtual void	inset(const rect& rc){
			_inset = rc;
		}
		virtual const rect& inset() const{
			return _inset;
		}

		virtual bool	visible() const {
			return _b_visible && _b_visible_by_parent;
		}

		virtual void visible(bool visible_){
			_b_visible = visible_;
			displayed(displayed());
			need_parent_update();
		}

		virtual void visible_by_parent(bool visible_){
			_b_visible_by_parent = visible_;
			displayed(displayed());
		}

		virtual bool displayed() const {
			return _b_displayed;
		}

		virtual void displayed(bool displayed_){
			_b_displayed = displayed_;
			if (::IsWindow(_hwnd)){
				::ShowWindow(_hwnd, visible() ? SW_SHOW : SW_HIDE);
			}
		}

		virtual csize estimate_size(const csize& available);

		virtual void attribute(LPCTSTR name, LPCTSTR value, bool inited = false);
        virtual void set_attr(LPCTSTR name, LPCTSTR value, bool inited = false) {
            return attribute(name, value, inited);
        }

		virtual void manager_(manager* mgr){
			_mgr = mgr;
		}

		virtual void hwnd(HWND hwnd_){
			_hwnd = hwnd_;
		}
		virtual HWND hwnd() const{
			return _hwnd;
		}
		operator HWND() const{
			return hwnd();
		}

		virtual void id(int id_){
			_id = id_;
		}
		virtual int id() const {
			return _id;
		}

		virtual void need_update(){
			if (_b_inited){
				pos(pos());
			}
		}

		virtual void need_parent_update();

		void user_data(void* ud){
			_ud = ud;
		}
		void* user_data() const {
			return _ud;
		}

		virtual container* parent() const{
			return reinterpret_cast<container*>(_parent);
		}
		virtual void parent(container* pa){
			_parent = reinterpret_cast<control*>(pa);
		}

		virtual void name(LPCTSTR n){
			_name = n;
		}
		virtual const string& name() const {
			return _name;
		}

		virtual control* find(LPCTSTR n);
		virtual control* find(HWND h);

	protected:
		string _name;
		int _id;
		HWND _hwnd;

		rect _pos;
		csize _post_size;

		int _width;
		int _height;
		int _min_width;
		int _max_width;
		int _min_height;
		int _max_height;

		rect _inset;

		bool _b_visible;
		bool _b_visible_by_parent;
		bool _b_displayed;

		manager* _mgr;
		control* _parent;

		bool _b_inited;

		int _font;

		void* _ud;
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

		virtual void visible(bool visible_) override;
		virtual void displayed(bool displayed_) override;

		virtual void manager_(manager* mgr) override;

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
	public:
		window_container();

	protected:
		virtual void init() override;
		void manager_(manager* mgr);
		virtual void attribute(LPCTSTR name, LPCTSTR value, bool inited = false) override;

	private:
		csize _init_size;
	};
}

#endif // __laywin_control_h__
