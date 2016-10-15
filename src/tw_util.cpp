#include "tw_util.h"

namespace taowin {

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