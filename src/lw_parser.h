#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "lw_util.h"
#include "lw_control.h"
#include "lw_syscontrols.h"
#include "lw_resmgr.h"

namespace laywin{
    namespace parser {
        class PARSER_OBJECT {
        public:
            void set_attr(const char* name, const char* value) {
                _attrs[name] = value;
            }

            void append_child(PARSER_OBJECT* c) {
                _children.push_back(c);
            }

        public:
            void dump_attr(std::function<void(const char* name, const char* value)> dumper) {
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

            std::string get_attr(const char* name, const char* def = "") {
                if(_attrs.count(name))
                    return _attrs[name];

                if(!def) def = "";
                return def;
            }

            bool has_attr(const char* name) {
                return _attrs.count(name) > 0;
            }

            void remove_attr(const char* name) {
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
            std::string tag;

        protected:
            std::map<std::string, std::string>  _attrs;
            std::vector<PARSER_OBJECT*>         _children;
        };

        PARSER_OBJECT* parse(char* xml, resmgr* mgr);
        PARSER_OBJECT* parse(const char* xml, resmgr* mgr);
    }
}
