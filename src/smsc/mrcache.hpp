#ifndef __SMSC_SYSTEM_MRCACHE_HPP__
#define __SMSC_SYSTEM_MRCACHE_HPP__

#include "core/synchronization/Mutex.hpp"
#include "sms/sms.h"
#include "util/int.h"

namespace smsc{

class MessageReferenceCache{
public:
  MessageReferenceCache()
  {
    memset(mrArray,0,sizeof(mrArray));
  }
  uint8_t getNextMR(const smsc::sms::Address& addr)
  {
    int idx=addr.length-5;
    if(idx<0)idx=0;
    idx=atoi(addr.value+idx);
    if(idx<0)idx=0;
    if(idx>=100000)idx=99999;
    smsc::core::synchronization::MutexGuard mg(mtx);
    return mrArray[idx]++;
  }
protected:
  uint8_t mrArray[100000];
  smsc::core::synchronization::Mutex mtx;
};

}//smsc

#endif
