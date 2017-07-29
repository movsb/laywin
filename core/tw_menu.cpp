#include <cassert>

#include <windows.h>

#include <functional>
#include <regex>

#include "tw_parser.h"
#include "tw_menu.h"

using namespace taowin::parser;

namespace taowin {

// 全局菜单ID分配
int MenuItem::_id = 0x0f;

MenuItem* MenuItem::_create_sib(string sid)
{
    auto sib = new MenuItem();

    sib->parent = this;
    sib->owner  = self;
    sib->sid    = sid;

    auto id = ++_id;
    sib->id = id;
    _idmap[id] = sib;

    return sib;
}

void MenuItem::_create_items(PARSER_OBJECT* c)
{
    MenuItem _dummy_child;
    MenuItem* prev_child = &_dummy_child;

    _dummy_child.next = nullptr;

    c->dump_children([&](PARSER_OBJECT* c) {
        auto sid        = c->get_attr(_T("id"));
        auto text       = c->get_attr(_T("text"));
        auto enable     = c->get_attr(_T("disabled")) != _T("1");
        auto key        = c->get_attr(_T("key"));
        auto checked    = c->get_attr(_T("checked")) == _T("1");

        auto sib = _create_sib(sid);

        prev_child->next = sib;
        sib->prev = prev_child;
        prev_child = sib;

        if(c->tag == _T("item")) {
            _insert_str(-1, sib->id, text, enable, key, checked);
        }
        else if(c->tag == _T("sep")) {
            _insert_sep(-1, sib->id);
        }
        else if(c->tag == _T("popup")) {
            HMENU hSubMenu = ::CreatePopupMenu();
            sib->self = hSubMenu;
            sib->_create_items(c);
            _insert_popup(hSubMenu, -1, sib->id, text, enable);
        }
    });

    child = _dummy_child.next;
    if(child) child->prev = nullptr;
}

void MenuItem::_insert_sep(UINT rid, UINT id) const
{
    MENUITEMINFO m {0};
    m.cbSize        = sizeof(m);
    m.fMask         = MIIM_ID;

    m.wID           = id;
    m.fType         = MFT_SEPARATOR;

    ::InsertMenuItem(self, rid, rid == -1, &m);
}

void MenuItem::_insert_popup(HMENU hSubMenu, UINT rid, UINT id, const string& s, bool enabled)
{
    MENUITEMINFO m {0};
    m.cbSize        = sizeof(m);
    m.fMask         = MIIM_ID | MIIM_STRING | MIIM_SUBMENU | MIIM_STATE;

    m.wID           = id;
    m.fType         = MFT_STRING;
    m.dwTypeData    = const_cast<LPTSTR>(s.c_str());
    m.hSubMenu      = hSubMenu;
    m.fState        = enabled ? MFS_ENABLED : MFS_GRAYED;

    ::InsertMenuItem(self, -1, TRUE, &m);
}

void MenuItem::_insert_str(UINT rid, UINT id, const string& text, bool enabled, const string& key, bool checked)
{
    MENUITEMINFO m {0};
    m.cbSize        = sizeof(m);
    m.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;

    m.wID           = id;
    m.fType         = MFT_STRING;

    // has hotkey
    string text_all = key.size() == 0 ? text : text + _T("\t") + key;
    m.dwTypeData    = const_cast<LPTSTR>(text_all.c_str());

    m.fState        = 0;
    m.fState       |= enabled ? MFS_ENABLED : MFS_GRAYED;
    m.fState       |= checked ? MFS_CHECKED : MFS_UNCHECKED;

    ::InsertMenuItem(self, rid, rid == -1, &m);
}

MenuItem* MenuItem::find_sib(const string& ids_)
{
    string ids = ids_ + _T('\0');
#ifdef _UNICODE
    std::wregex re(L"[,:]");
#else
    std::regex re("[,:]");
#endif
    ids = std::regex_replace(ids, re, string(1, 0));

    auto p = this;
    const TCHAR* s = ids.c_str();

    while(*s) {
        p = p->child;

        while(p && p->sid != s)
            p = p->next;

        if(!p) break;

        while(*s++)
            ;
    }

    return p;
}

void MenuItem::insert_str(const string& ref, const string& sid, const string& s, bool enabled, const string& key, bool checked)
{
    _insert_item(ref, sid, [&](UINT rid, UINT id) {
        _insert_str(rid, id, s, enabled, key, checked);
    });
}

void MenuItem::insert_sep(const string& ref, const string& sid)
{
    _insert_item(ref, sid, [&](UINT rid, UINT id) {
        _insert_sep(rid, id);
    });
}

void MenuItem::_init()
{
    parent = nullptr;
    prev = nullptr;
    next = nullptr;
    child = nullptr;
    id = 0;
    owner = nullptr;
    ud = 0;
}

void MenuItem::_link(MenuItem* p1, MenuItem* p2, MenuItem* p3)
{
    if(p1 != nullptr) {
        p1->next = p2;
    }

    p2->prev = p1;
    p2->next = p3;

    if(p3 != nullptr) {
        p3->prev = p2;
    }
}

void MenuItem::_insert_item(const string& ref, const string& sid, std::function<void(UINT, UINT)> callback)
{
        if(ref == _T("")) {
        auto sib = _create_sib(sid);
        if(child) {
            auto next = child->next;
            while(next && next->next)
                next = next->next;
            _link(next, sib, nullptr);
            _insert_sep(-1, sib->id);
            callback(-1, sib->id);
        }
        else {
            child = sib;
            callback(-1, sib->id);
        }
    }
    else {
        auto rsib = find_sib(ref);
        if(rsib != nullptr) {
            auto sib = _create_sib(sid);
            if(rsib->prev == nullptr) {
                child = sib;
            }
            _link(rsib->prev, sib, rsib);
            callback(rsib->id, sib->id);
        }
    }
}

std::vector<string> MenuItem::get_ids(int id) const
{
    std::vector<string> ids;

    auto it = _idmap.find(id);
    if(it != _idmap.cend()) {
        auto s = it->second;

        while(s) {
            ids.emplace_back(s->sid);
            s = s->parent;
        }
    }

    return std::move(ids);
}

MenuItem* MenuItem::get_popup(int id) const
{
    auto it = _idmap.find(id);
    if(it == _idmap.cend())
        return nullptr;

    auto sib = it->second;
    if(!sib->is_popup())
        return nullptr;

    return it->second;
}

MenuItem* MenuItem::match_popup(const string& ids, HMENU popup)
{
    auto sib = find_sib(ids);
    if(sib && sib->self == popup)
        return sib;
    return nullptr;
}

MenuItem* MenuItem::create(const TCHAR* xml)
{
    PARSER_OBJECT* root = parser::parse(xml, nullptr);
    if(!root || root->tag != _T("MenuTree")) {
        return nullptr;
    }

    auto sib = new MenuItem();
    sib->self = ::CreatePopupMenu();
    sib->sid = root->get_attr(_T("id"));
    sib->_create_items(root);

    return sib;
}

void MenuItem::track(const POINT* pt_, HWND owner)
{
    if(!is_popup()) {
        return;
    }

    POINT pt;
    POINT const *ppt = pt_;
    if(!ppt) {
        ::GetCursorPos(&pt);
        ppt = &pt;
    }

    ::TrackPopupMenu(self, TPM_LEFTBUTTON | TPM_LEFTALIGN, ppt->x, ppt->y, 0, owner, nullptr);
}

void MenuItem::clear()
{
    if(is_popup()) {
        int count = ::GetMenuItemCount(self);
        if(count != -1 && count > 0) {
            while(--count >= 0) {
                ::DeleteMenu(self, count, MF_BYPOSITION);
            }
        }
    }
}

bool MenuItem::is_popup()
{
    return self != nullptr;
}

bool MenuItem::is_enabled()
{
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    if(::GetMenuItemInfo(owner, id, FALSE, &mii)) {
        return !(mii.fState & MFS_DISABLED);
    }
    return false;
}

void MenuItem::enable(int enable)
{
    if(enable == -1) enable = is_enabled() ? 0 : 1;
    int state = enable == 1 ? MF_ENABLED : MF_GRAYED;
    ::EnableMenuItem(owner, id, state);
}

bool MenuItem::is_checked()
{
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    if(::GetMenuItemInfo(owner, id, FALSE, &mii)) {
        return !!(mii.fState & MFS_CHECKED);
    }
    return false;
}

void MenuItem::check(int check)
{
    if(is_popup()) {
        return;
    }

    if(check == -1) check = is_checked() ? 0 : 1;
    int state = check == 1 ? MF_CHECKED : MF_UNCHECKED;

    ::CheckMenuItem(parent->self, id, MF_BYCOMMAND | state);
}

void MenuItem::remove()
{
    ::DeleteMenu(owner, id, MF_BYCOMMAND);
}

}
