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
extern std::string dump(unsigned short len, unsigned char* udp, bool ascii = false);
extern std::string getBufferDump(const char* pBuffer, int nLength);
extern std::string format(const char* szFormat, ... );
extern void dumpToLog(Logger* logger, int len, const unsigned char* buffer );

}//namespace common
}//namespace inman
}//namespace smsc
#endif /* __SMSC_INMAN_COMMON_UTIL_HPP__ */

