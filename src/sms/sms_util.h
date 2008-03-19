#ifndef __SMSC_SMS_SMS_UTIL_H__
#define __SMSC_SMS_SMS_UTIL_H__

#include "sms.h"
//#include "core/buffers/File.hpp"
#include "util/int.h"

namespace smsc{
namespace sms{

template <class T>
inline void ReadAddress(T& f,Address& addr)
{
  addr.type=f.ReadByte();
  addr.plan=f.ReadByte();
  f.Read(addr.value,sizeof(addr.value));
  addr.value[sizeof(addr.value)-1]=0;
  addr.length=(uint8_t)strlen(addr.value);
}

template <class T>
inline void WriteAddress(T& f,const Address& addr)
{
  f.WriteByte(addr.type);
  f.WriteByte(addr.plan);
  AddressValue buf={0,};
  memcpy(buf,addr.value,addr.length);
  f.Write(buf,sizeof(buf));
}

inline uint32_t AddressSize()
{
  return 1+1+(int)sizeof(AddressValue);
}

}//sms
}//smsc
#endif
