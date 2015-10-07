#ifndef __laywin_parser_h__
#define __laywin_parser_h__

#include "lw_util.h"
#include "lw_control.h"
#include "lw_syscontrols.h"
#include "lw_manager.h"

namespace laywin{
	class parser {
	public:
		container* parse(LPCTSTR json, manager* mgr);

	private:
		void _parse_meta();
		void _parse_meta_font();
		void _parse_control(container* parent);

	private:
		inline bool _valid_ptr();
		inline bool _valid_suffix();
		void _skip_ws();
		string _read_key();
		string _read_value();
		void _skip_to_value();
		void _skip_to_next();


	private:
		manager* _manager;
		container* _container;
		LPCTSTR _p;
		int _line;
	};
}

#endif //__laywin_parser_h__
