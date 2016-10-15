#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <tchar.h>

namespace taowin{
	class csize
	{
	public:
		int cx;
		int cy;
	};

	class Rect : public tagRECT
	{
	public:
		Rect(){
			empty();
		}
		Rect(const tagRECT& r){
			left    = r.left;
			top     = r.top;
			right   = r.right;
			bottom  = r.bottom;
		}
		Rect(int l, int t, int r, int b){
			left    = l;
			top     = t;
			right   = r;
			bottom  = b;
		}

		void empty(){
			left = top = right = bottom = 0;
		}

		int width() const {
			return right - left;
		}

		int height() const {
			return bottom - top;
		}

		void join(const tagRECT& rc){
			if (rc.left     < left)     left    = rc.left;
			if (rc.top      < top)      top     = rc.top;
			if (rc.right    > right)    right   = rc.right;
			if (rc.bottom   > bottom)   bottom  = rc.bottom;
		}

		void offset(int cx, int cy){
			::OffsetRect(this, cx, cy);
		}

		void inflate(int cx, int cy){
			::InflateRect(this, cx, cy);
		}

		void deflate(int cx, int cy){
			::InflateRect(this, -cx, -cy);
		}

		void union_(const tagRECT& rc){
			::UnionRect(this, this, &rc);
		}
	};

#ifdef _UNICODE
	typedef std::wstring string;
#else
	typedef std::string string;
#endif

	template<class T>
	class array
	{
	public:
		array(int size = 0)
			: _pt(NULL)
			, _count(0)
			, _alloc(0)
		{
			if (size > 0){
				_pt = new T[size];
			}
		}
		~array(){
			clear();
		}

		void clear(){
			if (_pt){
				delete[] _pt;
				_pt = NULL;
			}
			_count = 0;
			_alloc = 0;
		}
		int find(const T& t) const{
			for (int i = 0; i < _count; ++i){
				if (_pt[i] == t){
					return i;
				}
			}
			return -1;
		}
		bool add(const T& t){
			if (++_count >= _alloc){
				int alloc = _alloc * 2;
				if (alloc == 0) alloc = 4;

				T* pt = new T[alloc];
				for(int i = 0; i < _count-1; ++i)
					pt[i] = _pt[i];
				delete[] _pt;
				_pt = pt;

				_alloc = alloc;
			}

			_pt[_count - 1] = t;
			return true;
		}
		bool remove(int i){
			if (i < --_count){
				int s = i;
				int e = s + (_count - i);
				for (int x = s; x < e; ++x){
					_pt[x] = _pt[x + 1];
				}
			}
			return true;
		}

		bool remove(const T& t){
			int i = find(t);
			if (i != -1){
				return remove(i);
			}
			return false;
		}

		int size() const{
			return _count;
		}
        bool empty() const {
            return _count == 0;
        }
		T& operator[](int i) const{
			return _pt[i];
		}

	protected:
		T*      _pt;
		int     _count;
		int     _alloc;
	};

    static void split_string(std::vector<string>* vec, LPCTSTR str, TCHAR delimiter=',')
	{
		LPCTSTR p = str;
		string tmp;
		for(;;){
			if(*p){
				if(*p != delimiter){
					tmp += *p;
					p++;
					continue;
				}
				else{
					vec->push_back(tmp);
					tmp = _T("");
					p++;
					continue;
				}
			}
			else{
				if(tmp.size()) vec->push_back(tmp);
				break;
			}
		}
	}

    string last_error();
}
