#ifndef __CRC32_H__
#define __CRC32_H__
#include "int.h"

namespace smsc{
namespace util{

uint32_t crc32(uint32_t crc,const void* buf,uint32_t len);

} //namespace util
} //namespace smsc

#endif
