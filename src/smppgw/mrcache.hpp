#ifndef __SMSC_SYSTEM_MRCACHE_HPP__
#define __SMSC_SYSTEM_MRCACHE_HPP__

#include "core/synchronization/Mutex.hpp"
#include "core/buffers/Hash.hpp"
#include "util/debug.h"

#include "sms/sms.h"

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif


namespace smsc{
namespace smppgw{

using smsc::core::buffers::Hash;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

using namespace smsc::sms;

class MessageReferenceCache{
public:
  MessageReferenceCache():cache(200000){}
  void setMR(const char* addr,uint16_t mr)
  {
    cache[addr]=mr;
  }
  uint16_t getNextMR(const Address& addr)
  {
    MutexGuard mg(mtx);
    char straddr[MAX_FULL_ADDRESS_VALUE_LENGTH+5];
    addr.toString(straddr,sizeof(straddr));
    uint16_t* dta=cache.GetPtr(straddr);
    if(dta)
    {
      return ++(*dta);
    }else
    {
      int rv=0;
      cache[straddr]=rv;
      return rv;
    }
  }
protected:
  Hash<uint16_t> cache;
  Mutex mtx;
};

}//smppgw
}//smsc

#endif
