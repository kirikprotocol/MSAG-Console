#ident "$Id$"
#ifndef __SMSC_INMAN_COMMON_UTIL_HPP__
#define __SMSC_INMAN_COMMON_UTIL_HPP__
#include <string>

#include "inman/common/types.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace common {

extern void pack_addr(unsigned char* dst, const char* src, int len);
extern void unpack_addr(char* dst, unsigned char* src, int len);
extern void fillAddress(SCCP_ADDRESS_T* dst, const char *saddr, unsigned char ssn);
extern void fillAddress(ADDRESS_BUF_T *dst, const char *saddr);

//uses internal 4k buffer for formatting
extern int         format(std::string & fstr, const char* fmt, ...);
extern std::string format(const char* szFormat, ... );

extern std::string dump(unsigned short len, unsigned char* udp, bool ascii = false);
extern int         dump(std::string& fstr, unsigned short size,
                                            unsigned char* buff, bool ascii);

extern void dumpToLog(Logger* logger, int len, const unsigned char* buffer );

}//namespace common
}//namespace inman
}//namespace smsc
#endif /* __SMSC_INMAN_COMMON_UTIL_HPP__ */

