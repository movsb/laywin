#pragma once

#include "tw_parser.h"

namespace taowin {

class MenuIDs
{
public:
    MenuIDs(std::vector<string>&& ids)
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

struct MenuInsertionType
{
    enum Value
    {
        Begin,
        Before,
        After,
        End,
    };
};

class MenuItem
{
public:
    MenuItem * _create_sib(string sid);

    void _create_items(taowin::parser::PARSER_OBJECT * c);

    void _insert_sep(UINT rid, UINT id) const;

    void _insert_popup(HMENU hSubMenu, UINT rid, UINT id, const string & s, bool enabled);

    void _insert_str(UINT rid, UINT id, const string & text, bool enabled, const string & key, bool checked);

    MenuItem * find_sib(const string & ids_);

    void insert_str(const string& ref, const string& sid, const string & s, bool enabled, const string & key, bool checked);

    std::vector<string> get_ids(int id) const;

    MenuItem * get_popup(int id) const;

    MenuItem * match_popup(const string & ids, HMENU popup);

    // �� xml �����˵�
    static MenuItem* create(const TCHAR* xml, bool main);

    // �����˲˵�
    void track(const POINT* pt = nullptr, HWND owner = ::GetActiveWindow());

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

    // �ı��ı�
    void set_text(const string& s);

    // ����һ���ָ���
    // pos ָ������λ�ù�ϵ
    // ref Ϊλ�òο���
    // sid Ϊ�������sid
    void insert_sep(const string& ref, const string& sid);

    // ȡ�þ��
    HMENU get_handle() const { return self; }

protected:
    void _init();
    void _link(MenuItem* p1, MenuItem* p2, MenuItem* p3);
    void _insert_item(const string& ref, const string& sid, std::function<void(UINT, UINT)> callback);

public:
    MenuItem()
    {
        _init();
    }

protected:
    static int _id;

    HMENU self;
    std::map<int, MenuItem*> _idmap;

    MenuItem* parent;    // �����
    MenuItem* prev;      // ǰһ�����
    MenuItem* next;      // ��һ�����
    MenuItem* child;     // ���ӽ�㣨�����˵����ã�

    UINT     id;        // �˵�ID��Windowsʶ��
    string   sid;       // �ַ���ID��taowinʶ��
    HMENU    owner;     // ���˵���Windowsʶ��
    void*    ud;        // �û�����
};

typedef MenuItem PopupMenu;

}
