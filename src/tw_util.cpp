#include "tw_util.h"

namespace taowin {

void split_string(std::vector<string>* vec, LPCTSTR str, TCHAR delimiter)
{
    LPCTSTR p = str;
    string tmp;
    for(;;) {
        if(*p) {
            if(*p != delimiter) {
                tmp += *p;
                p++;
                continue;
            }
            else {
                vec->push_back(tmp);
                tmp = _T("");
                p++;
                continue;
            }
        }
        else {
            if(tmp.size()) vec->push_back(tmp);
            break;
        }
    }
}

string last_error()
{
    string r;
    TCHAR* b = _T("");

    if (::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPTSTR)&b, 1, NULL)) {
        r = b;
        ::LocalFree(b);
    }

    return std::move(r);
}

}