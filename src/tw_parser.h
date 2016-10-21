/* taoxml: the simple, stupid xml parser
 *  written by: movsb
 *  written in: Oct 11, 2015
 *  https://github.com/movsb/taoxml
 **/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "tw_util.h"

namespace taowin{
    namespace parser {
        class PARSER_OBJECT {
        public:
            void set_attr(const TCHAR* name, const TCHAR* value) {
                _attrs[name] = value;
            }

            void append_child(PARSER_OBJECT* c) {
                _children.push_back(c);
            }

        public:
            void dump_attr(std::function<void(const TCHAR* name, const TCHAR* value)> dumper) {
                auto it = _attrs.cbegin();
                while(it != _attrs.cend()) {
                    dumper(it->first.c_str(), it->second.c_str());
                    it++;
                }
            }

            void dump_children(std::function<void(PARSER_OBJECT* c)> dumper) {
                auto it = _children.cbegin();
                while(it != _children.cend()) {
                    dumper(*it);
                    it++;
                }
            }

            string get_attr(const TCHAR* name, const TCHAR* def = _T("")) {
                if(_attrs.count(name))
                    return _attrs[name];

                if(!def) def = _T("");
                return def;
            }

            bool has_attr(const TCHAR* name) {
                return _attrs.count(name) > 0;
            }

            void remove_attr(const TCHAR* name) {
                _attrs.erase(name);
            }

            int count_attrs() const {
                return (int)_attrs.size();
            }

            std::map<string, string>& attrs() {
                return _attrs;
            }

            int count_children() const {
                return (int)_children.size();
            }

            PARSER_OBJECT* first_child() const {
                if(count_children())
                    return _children[0];
                return nullptr;
            }

        public:
            string tag;

        protected:
            std::map<string, string>  _attrs;
            std::vector<PARSER_OBJECT*>         _children;
        };

        PARSER_OBJECT* parse(TCHAR* xml, const TCHAR** err);
        PARSER_OBJECT* parse(const TCHAR* xml, const TCHAR** err);
    }
}
