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
using smsc::store::MessageStore;

class MessageReferenceCache{
public:
  MessageReferenceCache():cache(200000),store(0){}
  void assignStore(MessageStore* st)
  {
    store=st;
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
      int rv=store->getConcatMessageReference(addr);
      rv=rv==-1?0:rv;
      cache[straddr]=rv;
      return rv;
    }
  }
protected:
  Hash<uint8_t> cache;
  Mutex mtx;
  MessageStore* store;
};

};//system
};//smsc

#endif
