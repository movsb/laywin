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

    // 从 xml 创建菜单
    static MenuItem* create(const TCHAR* xml, bool main);

    // 弹出此菜单
    void track(const POINT* pt = nullptr, HWND owner = ::GetActiveWindow());

    // 如果是弹出菜单，清空其内容
    void clear();

    // 判断是否是弹出菜单
    bool is_popup();

    // 是否启用
    bool is_enabled();

    // 启用与禁用
    void enable(int enable);

    // 是否打勾
    bool is_checked();

    // 打勾与取消
    void check(int check);

    // 从父结点中删除此菜单
    void remove();

    // 改变文本
    void set_text(const string& s);

    // 插入一个分隔符
    // pos 指定插入位置关系
    // ref 为位置参考点
    // sid 为插入项的sid
    void insert_sep(const string& ref, const string& sid);

    // 取得句柄
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

    MenuItem* parent;    // 父结点
    MenuItem* prev;      // 前一个结点
    MenuItem* next;      // 后一个结点
    MenuItem* child;     // 孩子结点（弹出菜单可用）

    UINT     id;        // 菜单ID（Windows识别）
    string   sid;       // 字符串ID（taowin识别）
    HMENU    owner;     // 父菜单（Windows识别）
    void*    ud;        // 用户数据
};

typedef MenuItem PopupMenu;

}
