#include <windows.h>
#include <tchar.h>

#include <string>
#include <vector>

#include "lw_parser.h"

namespace laywin{

	container* parser::parse(LPCTSTR json, manager* mgr)
	{
		_manager = mgr;
		_p = json;
		_line = 1;
		_container = NULL;
		if(!_p || !*_p) return NULL;

		_skip_ws();
		if(*_p++ != _T('{'))
			throw _T("expect  `{'");
		if(*_p == _T('}')){
			_p++;
			return _container;
		}

		for(;;){
			string key = _read_key();
			_skip_to_value();

			if(key == _T("meta")){
				_parse_meta();
			}
			else if(key == _T("window")){
				_parse_control(NULL);
			}

			_skip_to_next();
			if(*_p == _T('}'))
				break;
		}

		_skip_ws();
		if(*_p++ != _T('}'))
			throw _T("expect `}'");

		return _container;
	}

	void parser::_skip_ws()
	{
		for(;;){
			if(*_p == _T(' ') || *_p == _T('\t')){
				++_p;
			}
			else if(*_p == _T('\r')){
				_p++;
				_line++;
				if(*_p == _T('\n'))
					_p++;
			}
			else if(*_p == _T('\n')){
				_p++;
				_line++;
			}
			else{
				break;
			}
		}

	}

	void parser::_parse_control(container* parent)
	{
		if(parent && !parent->is_container())
			throw _T("expect container");

		_skip_ws();
		if(*_p++ != _T('{'))
			throw _T("expect `{'");

		_skip_ws();
		if(*_p == _T('}')){
			_p++;
			return;
		}

		control* pcon = NULL;
		if(!parent){
			_container = new window_container;
			pcon = _container;
		}
		else{
			string type = _read_key();
			if(type != _T("type"))
				throw _T("`type' expected");
			_skip_to_value();

			string con = _read_value();
			if(con == _T("window"))
				throw _T("`window' not expected");

			if(con == _T("control"))			pcon = new control;
			else if(con == _T("container"))		pcon = new container;
			else if(con == _T("horizontal"))	pcon = new horizontal;
			else if(con == _T("vertical"))		pcon = new vertical;

			else if(con == _T("button"))		pcon = new button;
			else if(con == _T("option"))		pcon = new option;
			else if(con == _T("check"))			pcon = new check;
			else if(con == _T("static"))		pcon = new static_;
			else if(con == _T("group"))			pcon = new group;
			else if(con == _T("edit"))			pcon = new edit;
			else if(con == _T("listview"))		pcon = new listview;

			else throw _T("unknown control type");

			parent->add(pcon);

			_skip_to_next();
			if(*_p == _T('}')){
				_p++;
				return;
			}
		}

		for(;;){
			string key = _read_key();
			_skip_to_value();

			if(key == _T("children")){
				if(*_p++ != _T('['))
					throw _T("expect `['");
				_skip_ws();
				if(*_p == _T(']')){
					_p++;
					break;
				}

				for(;;){
					_parse_control(static_cast<container*>(pcon));
					_skip_to_next();
					if(*_p == _T(']'))
						break;
				}

				if(*_p++ != _T(']'))
					throw _T("expect `]'");
			}
			else{
				string val = _read_value();
				pcon->attribute(key.c_str(), val.c_str());
			}

			_skip_to_next();
			if(*_p == _T('}'))
				break;
		}

		_skip_ws();
		if(*_p++ != _T('}'))
			throw _T("expect `}'");
	}

	string parser::_read_key()
	{
		_skip_ws();
		if(*_p++ != _T('"'))
			throw _T("key expect start tag `\"\'");

		string key;
		while(_valid_ptr() && *_p != _T('\"')){
			key += *_p;
			_p++;
		}
		if(!_valid_ptr())
			throw _T("key expect end tag `\"\'");
		_p++;

		return key;
	}

	void parser::_skip_to_value()
	{
		_skip_ws();
		if(*_p++ != _T(':'))
			throw _T("expect `:'");
		_skip_ws();
		if(!_valid_ptr())
			throw _T("value expected");
	}

	bool parser::_valid_ptr()
	{
		return *_p != 0;
	}

	laywin::string parser::_read_value()
	{
		_skip_ws();
		if(*_p == _T('"')){
			string key = _read_key();
			if(!_valid_suffix())
				throw _T("invalid suffix after `string'");
			return key;
		}
		else if(*_p==_T('t') || *_p==_T('f')){
			if(_tcsncmp(_p, _T("true"), 4) == 0){
				_p += 4;
				if(!_valid_suffix())
					throw _T("invalid suffix after `true'");
				return _T("true");
			}
			else if(_tcsncmp(_p, _T("false"), 5) == 0){
				_p += 5;
				if(!_valid_suffix())
					throw _T("invalid suffix after `false'"); 
				return _T("false");
			}
			else{
				throw _T("invalid value");
			}
		}
		else if(*_p >= _T('0') && *_p <= _T('9')){
			string val;
			while(*_p >= _T('0') && *_p <= _T('9')){
				val += *_p;
				_p++;
			}
			if(!_valid_suffix())
				throw _T("invalid suffix after `integer'");
			return val;
		}
		else{
			throw _T("invalid value");
		}
	}

	void parser::_skip_to_next()
	{
		_skip_ws();
		if(*_p == _T(']') || *_p == _T('}'))
			return;

		if(*_p++ != _T(','))
			throw _T("expect `,'");

		_skip_ws();
	}

	bool parser::_valid_suffix()
	{
		_skip_ws();
		return *_p == _T(',')
			|| *_p == _T(']') 
			|| *_p == _T('}');
	}

	void parser::_parse_meta()
	{
		_skip_ws();
		if(*_p++ != _T('{'))
			throw _T("expect `{'");
		_skip_ws();
		if(*_p == _T('}')){
			_p++;
			return;
		}

		for(;;){
			string key = _read_key();
			_skip_to_value();

			if(key == _T("font")){
				_parse_meta_font();
			}

			_skip_to_next();
			if(*_p == _T('}'))
				break;
		}

		_skip_ws();
		if(*_p++ != _T('}'))
			throw _T("expect `}'");
	}

	void parser::_parse_meta_font()
	{
		_skip_ws();
		if(*_p++ != _T('['))
			throw _T("expect `['");
		_skip_ws();
		if(*_p == _T(']')){
			_p++;
			return;
		}

		for(;;){
			_skip_ws();
			if(*_p++ != _T('{'))
				throw _T("expect `{'");
			_skip_ws();
			if(*_p == _T('}')){
				_p++;
				return;
			}

			string font_family;
			string font_size;
			bool font_default=false;

			for(;;){
				string key = _read_key();
				_skip_to_value();

				if(key == _T("family")){
					font_family = _read_value();
				}
				else if(key == _T("size")){
					font_size = _read_value();
				}
				else if(key == _T("default")){
					font_default = _read_value() == _T("true");
				}

				_skip_to_next();
				if(*_p == _T('}'))
					break;
			}

			_manager->add_font(font_family.c_str(), _tstoi(font_size.c_str()), false, false, false);
			if(font_default) _manager->default_font(font_family.c_str(), _tstoi(font_size.c_str()), false, false, false);

			_skip_ws();
			if(*_p++ != _T('}'))
				throw _T("expect `}'");

			_skip_to_next();
			if(*_p == _T(']'))
				break;
		}

		_skip_ws();
		if(*_p++ != _T(']'))
			throw _T("expect `]'");
	}
}
