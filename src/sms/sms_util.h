#ifndef __SMSC_SMS_SMS_UTIL_H__
#define __SMSC_SMS_SMS_UTIL_H__

#include "sms.h"
#include "core/buffers/File.hpp"
#include "util/int.h"

namespace smsc{
namespace sms{

inline void ReadAddress(smsc::core::buffers::File& f,Address& addr)
{
  addr.type=f.ReadByte();
  addr.plan=f.ReadByte();
  f.Read(addr.value,sizeof(addr.value));
  addr.value[sizeof(addr.value)-1]=0;
  addr.length=strlen(addr.value);
}

inline void WriteAddress(smsc::core::buffers::File& f,const Address& addr)
{
  f.WriteByte(addr.type);
  f.WriteByte(addr.plan);
  AddressValue buf={0,};
  strcpy(buf,addr.value);
  f.Write(buf,sizeof(buf));
}

inline uint32_t AddressSize()
{
  return 1+1+sizeof(AddressValue);
}

}//sms
}//smsc
#endif
