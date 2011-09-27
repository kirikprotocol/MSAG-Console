#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <stdio.h>
#include "util/vformat.hpp"

namespace smsc {
namespace util {

#ifndef VSNPRINTF
#ifdef _WIN32
#define VSNPRINTF _vsnprintf
#else
#define VSNPRINTF vsnprintf
#endif
#endif /* VSNPRINTF */

unsigned vpformat(const char* fmt, va_list arg_list, char ** p_buf,
                 unsigned buf_sz/* = 1024*/)
{
    char * vbuf = new char[buf_sz];
    va_list aq;
    va_copy(aq,arg_list);
    int n = VSNPRINTF(vbuf, buf_sz - 1, fmt, aq);
    va_end(aq);
    if ((n > 0) && ((unsigned)n >= buf_sz)) {
        buf_sz = n + 2; // + '\0'
        delete [] vbuf;
        vbuf = new char[buf_sz];
        va_copy(aq,arg_list);
        n = VSNPRINTF(vbuf, buf_sz - 1, fmt, aq);
        va_end(aq);
    }
    vbuf[(n >= 0) ? n : 0] = 0; //vsnprintf() may return -1 on error
    *p_buf = vbuf;
    return n;
}

std::string & vformat(std::string & fstr, const char* fmt, va_list arg_list)
{
    char abuf[1024]; abuf[0] = 0;
    va_list aq;
    va_copy(aq,arg_list);
    int n = VSNPRINTF(abuf, sizeof(abuf) - 1, fmt, aq);
    va_end(aq);
    if (n > 0) {
      if ((unsigned)n >= sizeof(abuf)) {
        char * vbuf = NULL;
        n = vpformat(fmt, arg_list, &vbuf, n + 2);
        fstr += vbuf;
        delete [] vbuf;
      } else
        fstr += abuf;
    }
    return fstr;
}
#undef VSNPRINTF

std::string format(const char* fmt, ...)
{
    std::string fstr;
    va_list arg_list;
    va_start(arg_list, fmt);
    vformat(fstr, fmt, arg_list);
    va_end(arg_list);
    return fstr;
}

std::string & format(std::string & fstr, const char* fmt, ...)
{
    va_list arg_list;
    va_start(arg_list, fmt);
    vformat(fstr, fmt, arg_list);
    va_end(arg_list);
    return fstr;
}

}//namespace util
}//namespace smsc

