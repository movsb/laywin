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

class MenuManager
{
public:
    class Sibling
    {
        friend class MenuManager;

    public:
        // ����ǵ����˵������������
        void clear();

        // �ж��Ƿ��ǵ����˵�
        bool is_popup();

        // �Ƿ�����
        bool is_enabled();

        // ���������
        void enable(int enable);

        // �Ƿ��
        bool is_checked();

        // ����ȡ��
        void check(int check);

        // �Ӹ������ɾ���˲˵�
        void remove();

    protected:
        Sibling* parent;
        Sibling* prev;
        Sibling* next;
        Sibling* child;

        UINT     id;
        string   sid;
        HMENU    owner;
        HMENU    self;
        void*    ud;
    };

public:
    MenuManager()
        : _hmenu(nullptr)
    {
    }

    ~MenuManager()
    {
        destroy();
    }

public:
    void create(const TCHAR* xml);
    void destroy();

    void track(const POINT* pt = nullptr, HWND owner = ::GetActiveWindow());

    std::vector<string> get_ids(int id) const;

    Sibling* get_popup(int id) const;
    Sibling* match_popup(const string& ids, HMENU popup);

    Sibling* find_sib(const string& ids);

    void insert_str(Sibling* popup, string sid, const string& s, bool enabled = true, const string& key = _T(""), bool checked = false);
    void insert_sep(Sibling* popup);

    void enable(const string& ids, int enable);
    void check(const string& sid, int check);

protected:
    Sibling* _create_sib(string sid, Sibling* parent, HMENU owner, Sibling* prev);
    void _create_items(HMENU hMenu, parser::PARSER_OBJECT* c, Sibling* rel);
    void _insert_sep(HMENU hMenu, UINT id) const;
    void _insert_popup(HMENU hMenu, HMENU hSubMenu, UINT id, const string& s, bool enalbed = true);
    void _insert_str(HMENU hMenu, UINT id, const string& s, bool enabled, const string& key, bool checked);

protected:
    Sibling* _alloc_sib();
    void _dealloc_sib(Sibling* sib);

protected:
    static int   _id;
    HMENU _hmenu;
    Sibling _root;
    std::map<int, Sibling*> _idmap;
};


}

