#ifndef __SMSC_SYSTEM_TEMP_STORE_H__
#define __SMSC_SYSTEM_TEMP_STORE_H__

#include "core/buffers/XHash.hpp"
#include "sms/sms_const.h"
#include "core/synchronization/Mutex.hpp"
#include <exception>

namespace smsc{
namespace system{

using namespace smsc::core::synchronization;
using smsc::core::buffers::XHash;
using smsc::sms::SMSId;
using smsc::sms::SMS;

class NoMessageException:public std::exception
{
public:
  const char* what()const throw()
  {
    return "No such message in temp store";
  }
};

class TemporaryStore{
public:
  TemporaryStore():store(200000){}

  void AddSms(SMSId id,const SMS& sms)
  {
    SMS *psms=new SMS(sms);
    MutexGuard g(mtx);
    store.Insert(id,psms);
  }

  void AddPtr(SMSId id,SMS* psms)
  {
    MutexGuard g(mtx);
    store.Insert(id,psms);
  }

  bool Get(SMSId id,SMS& sms)
  {
    MutexGuard g(mtx);
    SMS** psms=store.GetPtr(id);
    if(!psms)return false;
    sms=**psms;
    return true;
  }

  SMS* Extract(SMSId id)
  {
    MutexGuard g(mtx);
    SMS** psms=store.GetPtr(id);
    if(!psms)return 0;
    store.Delete(id);
    return *psms;
  }

  void Delete(SMSId id)
  {
    MutexGuard g(mtx);
    SMS** psms=store.GetPtr(id);
    if(psms)
    {
      delete *psms;
      store.Delete(id);
    }
  }

protected:
  struct SMSIdHashFunc{
  static int CalcHash(const SMSId& id)
  {
    return (int)id;
  }
  };
  XHash<SMSId,SMS*,SMSIdHashFunc> store;
  Mutex mtx;
};

};//namespace system
};//namespace smsc

#endif
