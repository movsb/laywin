#pragma once

#include "tw_parser.h"

namespace taowin {

class MenuIds
{
public:
    MenuIds(std::vector<string>&& ids)
        : ids(ids)
    { }

    const string& operator[](size_t i) const
    {
        if(i < ids.size())
            return ids[ids.size()-1-i];

        static string _dummy;
        return _dummy;
    }

    std::vector<string>* operator->()
    {
        return &ids;
    }

protected:
    std::vector<string> ids;
};

class menu_manager
{
    struct sibling
    {
        sibling* parent;
        sibling* prev;
        sibling* next;
        sibling* child;

        UINT     id;
        string   sid;
        HMENU    owner;
    };

public:
    menu_manager()
        : _hmenu(nullptr)
    {
    }

    ~menu_manager()
    {
        destroy();
    }

public:
    void create(const TCHAR* xml);
    void destroy();
    void track(const POINT* pt = nullptr, HWND owner = ::GetActiveWindow());
    std::vector<string> get_ids(int id) const;
    void enable(const string& ids, bool b) const;

protected:
    void _create_items(HMENU hMenu, parser::PARSER_OBJECT* c, sibling* rel);
    void _insert_sep(HMENU hMenu, UINT id) const;
    void _insert_sub(HMENU hMenu, HMENU hSubMenu, UINT id, const string& s, bool enalbed = true);
    void _insert_str(HMENU hMenu, UINT id, const string& s, bool enabled = true);
    const sibling* find_sib(const string& ids) const;

protected:
    static int   _id;
    HMENU _hmenu;
    sibling _root;
    std::map<int, sibling*> _idmap;
};


}

