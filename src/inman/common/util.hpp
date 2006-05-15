#ident "$Id$"
#ifndef __SMSC_INMAN_COMMON_UTIL_HPP__
#define __SMSC_INMAN_COMMON_UTIL_HPP__

#include <string>

namespace smsc {
namespace inman {
namespace common {

//uses internal 4k buffer for formatting
extern int         format(std::string & fstr, const char* fmt, ...);
extern std::string format(const char* szFormat, ... );

extern std::string dump(unsigned short len, unsigned char* udp, bool ascii = false);
extern int         dump(std::string& fstr, unsigned short size,
                                            unsigned char* buff, bool ascii);

}//namespace common
}//namespace inman
}//namespace smsc
#endif /* __SMSC_INMAN_COMMON_UTIL_HPP__ */

