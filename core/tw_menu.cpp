#include <cassert>

#include <windows.h>

#include <functional>
#include <regex>

#include "tw_parser.h"
#include "tw_menu.h"

using namespace taowin::parser;

namespace taowin {

int MenuManager::_id = 0x0f;

MenuManager::Sibling* MenuManager::_create_sib(string sid, Sibling* parent, HMENU owner, Sibling* prev)
{
    auto sib = _alloc_sib();

    sib->parent = parent;
    sib->owner  = owner;
    sib->self   = nullptr;
    sib->child  = nullptr;
    sib->next   = nullptr;
    sib->prev   = prev;
    sib->sid    = sid;
    sib->ud     = nullptr;

    if(prev) prev->next = sib;

    auto id = ++_id;
    sib->id = id;
    _idmap[id] = sib;

    return sib;
}

void MenuManager::_create_items(HMENU hMenu, PARSER_OBJECT* c, Sibling* rel)
{
    rel->prev = nullptr;
    rel->next = nullptr;
    rel->child = nullptr;

    Sibling _dummy_child;
    Sibling* prev_child = &_dummy_child;

    _dummy_child.next = nullptr;

    c->dump_children([&](PARSER_OBJECT* c) {
        auto sid = c->get_attr(_T("i"));
        auto s = c->get_attr(_T("s"));
        auto e= c->get_attr(_T("d")) != _T("1");

        auto sib = _create_sib(sid, rel, hMenu, prev_child);

        prev_child = sib;

        if(c->tag == _T("item")) {
            _insert_str(hMenu, sib->id, s, e);
        }
        else if(c->tag == _T("sep")) {
            _insert_sep(hMenu, sib->id);
        }
        else if(c->tag == _T("sub")) {
            HMENU hSubMenu = ::CreatePopupMenu();
            sib->self = hSubMenu;
            _create_items(hSubMenu, c, sib);
            _insert_sub(hMenu, hSubMenu, sib->id, s, e);
        }
    });

    rel->child = _dummy_child.next;
    if(rel->child) rel->child->prev = nullptr;
}

void MenuManager::_insert_sep(HMENU hMenu, UINT id) const
{
    MENUITEMINFO m {0};
    m.cbSize        = sizeof(m);
    m.fMask         = MIIM_ID;

    m.wID           = id;
    m.fType         = MFT_SEPARATOR;

    ::InsertMenuItem(hMenu, -1, TRUE, &m);
}

void MenuManager::_insert_sub(HMENU hMenu, HMENU hSubMenu, UINT id, const string& s, bool enabled)
{
    MENUITEMINFO m {0};
    m.cbSize        = sizeof(m);
    m.fMask         = MIIM_ID | MIIM_STRING | MIIM_SUBMENU | MIIM_STATE;

    m.wID           = id;
    m.fType         = MFT_STRING;
    m.dwTypeData    = const_cast<LPTSTR>(s.c_str());
    m.hSubMenu      = hSubMenu;
    m.fState        = enabled ? MFS_ENABLED : MFS_GRAYED;

    ::InsertMenuItem(hMenu, -1, TRUE, &m);
}

void MenuManager::_insert_str(HMENU hMenu, UINT id, const string& s, bool enabled)
{
    MENUITEMINFO m {0};
    m.cbSize        = sizeof(m);
    m.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;

    m.wID           = id;
    m.fType         = MFT_STRING;
    m.dwTypeData    = const_cast<LPTSTR>(s.c_str());
    m.fState        = enabled ? MFS_ENABLED : MFS_GRAYED;

    ::InsertMenuItem(hMenu, -1, TRUE, &m);
}

MenuManager::Sibling* MenuManager::_alloc_sib()
{
    return new Sibling;
}

void MenuManager::_dealloc_sib(Sibling* sib)
{
    delete sib;
}

MenuManager::Sibling* MenuManager::find_sib(const string& ids_)
{
    string ids = ids_ + _T('\0');
#ifdef _UNICODE
    std::wregex re(L"[,:]");
#else
    std::regex re("[,:]");
#endif
    ids = std::regex_replace(ids, re, string(1, 0));

    auto p = &_root;
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

void MenuManager::insert_str(Sibling* popup, string sid, const string& s, bool enabled)
{
    if(!popup) popup = &_root;
    auto sib = _create_sib(sid, popup, popup->self, nullptr);
    _insert_str(popup->self, sib->id, s, enabled);
}

void MenuManager::insert_sep(Sibling* popup)
{
    if(!popup) popup = &_root;
    auto sib = _create_sib(_T(""), popup, popup->self, nullptr);
    _insert_sep(popup->self, 0);
}

void MenuManager::set_check(string ids, bool check)
{
    auto sib = find_sib(ids);
    if(sib->parent && sib->parent->self) {
        ::CheckMenuItem(sib->parent->self, sib->id, MF_BYCOMMAND | (check ? MF_CHECKED : MF_UNCHECKED));
    }
}

void MenuManager::create(const TCHAR* xml)
{
    PARSER_OBJECT* root = parser::parse(xml, nullptr);
    if(!root || root->tag != _T("MenuTree")) return;

    _hmenu = ::CreatePopupMenu();
    _root.parent = nullptr;
    _root.owner = nullptr;
    _root.self = _hmenu;
    _root.sid = root->get_attr(_T("i"));
    _create_items(_hmenu, root, &_root);
}

void MenuManager::destroy()
{
    if(_hmenu) {
        ::DestroyMenu(_hmenu);
        _hmenu = nullptr;
    }
}

void MenuManager::track(const POINT* pt_, HWND owner)
{
    POINT pt;
    POINT const *ppt = pt_;
    if(!ppt) {
        ::GetCursorPos(&pt);
        ppt = &pt;
    }

    ::TrackPopupMenu(_hmenu, TPM_LEFTBUTTON | TPM_LEFTALIGN, ppt->x, ppt->y, 0, owner, nullptr);
}

std::vector<string> MenuManager::get_ids(int id) const
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

void MenuManager::enable(const string& ids, bool b)
{
    if(auto sib = find_sib(ids)) {
        ::EnableMenuItem(sib->owner, sib->id, b ? MF_ENABLED : MF_GRAYED);
    }
}

MenuManager::Sibling * MenuManager::get_popup(int id) const
{
    auto it = _idmap.find(id);
    if(it == _idmap.cend())
        return nullptr;

    return it->second;
}

MenuManager::Sibling* MenuManager::match_popup(const string& ids, HMENU popup)
{
    auto sib = find_sib(ids);
    if(sib && sib->self == popup)
        return sib;
    return nullptr;
}

void MenuManager::clear_popup(Sibling* sib)
{
    if(sib && sib->self) {
        int count = ::GetMenuItemCount(sib->self);
        if(count != -1 && count > 0) {
            while(--count >= 0) {
                ::DeleteMenu(sib->self, count, MF_BYPOSITION);
            }
        }
    }
}

}
