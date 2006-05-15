static char const ident[] = "$Id$";

#include <stdarg.h>

#include "inman/common/util.hpp"

namespace smsc {
namespace inman {
namespace common {

int format(std::string & fstr, const char* fmt, ...)
{
    char format_buffer[4096];

    va_list va;
    va_start(va, fmt);
#ifdef _WIN32
    int u = _vsnprintf
#else
    int u = vsnprintf
#endif
            (format_buffer, sizeof(format_buffer) - 1, fmt, va);
    va_end(va);
    format_buffer[u] = 0;
    fstr += format_buffer;
    return u;
}

std::string format(const char* fmt, ...)
{
    char format_buffer[4096];

    va_list va;
    va_start(va, fmt);
#ifdef _WIN32
    int u = _vsnprintf
#else
    int u = vsnprintf
#endif
            (format_buffer, sizeof(format_buffer) - 1, fmt, va);
    va_end(va);
    format_buffer[u] = 0;
    return std::string(format_buffer);
}

int dump(std::string& fstr, unsigned short size, unsigned char* buff, bool ascii)
{
    int chars = 0;
    for (int i = 0; i < size; i++) {
        char tmp[32];
        unsigned char ch = buff[i];

        if ((ch < 32) || (ch > 127) || !ascii)
            chars += sprintf(tmp, "0x%02X ", ch);
        else 
            chars += sprintf(tmp, "'%c' ", ch);
        fstr += tmp;
    }
    if (size)
        fstr.erase(fstr.end()); //erase last space char
    return chars;
}

std::string dump(unsigned short size, unsigned char* buff, bool ascii)
{
    std::string reply;
    dump(reply, size, buff, ascii);
    return reply;
}

}//namespace common
}//namespace inman
}//namespace smsc

