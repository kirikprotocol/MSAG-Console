#ifndef __SMSC_SYSTEM_MRCACHE_HPP__
#define __SMSC_SYSTEM_MRCACHE_HPP__

#include "core/synchronization/Mutex.hpp"
#include "core/buffers/Hash.hpp"
#include "store/MessageStore.h"
#include "util/debug.h"

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif


namespace smsc{
namespace system{

using smsc::core::buffers::Hash;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::store::ConcatDataIterator;
using smsc::store::MessageStore;

class MessageReferenceCache{
public:
  MessageReferenceCache():cache(200000){}
  void loadFromStore(MessageStore* ms)
  {
    try{
      ConcatDataIterator *it=ms->getConcatInitInfo();
      while(it->getNext())
      {
        cache[it->getDestination()]=it->getMessageReference();
      }
      delete it;
    }catch(...)
    {
      __warning__("Failed to init mrcache");
    }
  }
  void setMR(const char* addr,uint8_t mr)
  {
    cache[addr]=mr;
  }
  uint8_t getNextMR(const Address& addr)
  {
    MutexGuard mg(mtx);
    AddressValue straddr;
    addr.toString(straddr,sizeof(straddr));
    uint8_t* dta=cache.GetPtr(straddr);
    if(dta)
    {
      return ++(*dta);
    }else
    {
      cache[straddr]=0;
      return 0;
    }
  }
protected:
  Hash<uint8_t> cache;
  Mutex mtx;
};

};//system
};//smsc

#endif
