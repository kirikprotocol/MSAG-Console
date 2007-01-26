#ifndef __SMSC_SYSTEM_MRCACHE_HPP__
#define __SMSC_SYSTEM_MRCACHE_HPP__

#include "core/synchronization/Mutex.hpp"
#include "sms/sms.h"
#include "util/int.h"

namespace smsc{
namespace system{

class MessageReferenceCache{
public:
  MessageReferenceCache()
  {
    memset(mrArray,0,sizeof(mrArray));
  }
  uint16_t getNextMR(const smsc::sms::Address& addr)
  {
    smsc::core::synchronization::MutexGuard mg(mtx);
    int idx=addr.length-4;
    if(idx<0)idx=0;
    idx=atoi(addr.value+idx);
    if(idx<0)idx=0;
    if(idx>=10000)idx=10000;
    return mrArray[idx]++;
  }
protected:
  uint8_t mrArray[10000];
  smsc::core::synchronization::Mutex mtx;
};

}//system
}//smsc

#endif
