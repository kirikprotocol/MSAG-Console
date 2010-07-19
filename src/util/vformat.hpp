/* ************************************************************************** *
 * Helpers: Formatted output to std::string.
 * ************************************************************************** */
#ifndef __SMSC_FORMAT_UTIL_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_FORMAT_UTIL_HPP__

#include <stdarg.h>
#include <string>

namespace smsc {
namespace util {

//prints formatted string to allocated buffer and returns its address and length
extern unsigned     vpformat(const char* fmt, va_list arg_list, char ** p_buf,
                                unsigned buf_sz = 1024);
//prints formatted arguments to given string using either buffer on stack or on heap
extern std::string& vformat(std::string & fstr, const char* fmt, va_list arg_list);

extern std::string& format(std::string & fstr, const char* fmt, ...);
extern std::string  format(const char* fmt, ... );

}//namespace util
}//namespace smsc
#endif /* __SMSC_INMAN_COMMON_UTIL_HPP__ */

