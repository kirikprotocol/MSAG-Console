/*
  $Id$
*/
#include "smeman.h"
#include "logger/Logger.h"
#include <stdexcept>
#include <string>

namespace smsc {
namespace smeman {

using namespace std;

using core::synchronization::MutexGuard;
#define __synchronized__ MutexGuard mguard(lock);

// ---- SmeAdministrator implementation --------------

void SmeManager::addSme(const SmeInfo& info)
{
__synchronized__
  auto_ptr<SmeRecord> record(new SmeRecord);
  record->info = info;
  record->info.internal=false;
  record->proxy = 0;
  record->deleted = false;
  record->idx = records.size();
  SmeIndex index = internalLookup(info.systemId);
  if ( index != INVALID_SME_INDEX ) throw runtime_error("Already exists");
  records.push_back(record.release());
}

void SmeManager::deleteSme(const SmeSystemId& systemId)
{
__synchronized__
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX ) throw SmeError();
  if ( records[index]->proxy )
  {
    // ???????  что делать если уже в работе , шутдаунить прокси, как корректно или абортом
    // к примеру так, но тогда , что делает close?
    dispatcher.detachSmeProxy(records[index]->proxy);
    //records[index]->proxy->close();
    records[index]->proxy->disconnect();
    //delete records[index].proxy;
    records[index]->proxy = 0;
  }
  records[index]->deleted = true;
}

/*void SmeManager::store()
{
}*/

class SmeIteratorImpl  : public SmeIterator
{
  Records::iterator begin;
  Records::iterator end;
  Records::iterator ptr;
  bool started;
public:
  SmeIteratorImpl(const Records::iterator& begin,const Records::iterator& end):
    begin(begin),end(end),ptr(begin),started(false) {}

  virtual bool next()
  {
    do{
      if (started&&ptr!=end) ++ptr;
      started = true;
    }while ( ptr!=end && (*ptr)->deleted );
    return ptr != end;
  }

  virtual SmeProxy* getSmeProxy() const
  {
    if ( ptr == end ) throw runtime_error("out of data");
    return (*ptr)->proxy;
  }
  virtual SmeInfo  getSmeInfo() const
  {
    if ( ptr == end ) throw runtime_error("out of data");
    return (*ptr)->info;
  }
  virtual SmeIndex getSmeIndex() const
  {
    if ( ptr == end ) throw runtime_error("out of data");
    return (*ptr)->idx;
  }
};

SmeIterator* SmeManager::iterator()
{
__synchronized__
  return new SmeIteratorImpl(records.begin(),records.end());
}

#if 0
void SmeManager::disableSme(const SmeSystemId& systemId)
{
  /*
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX ) throw SmeError();
  // ???????  что делать если уже в работе , шутдаунить прокси, как корректно или абортом
  records[index].info.disabled = true;*/
  __warning__("disableSme is not implemented");
}

void SmeManager::enableSme(const SmeSystemId& systemId)
{
  /*SmeIndex index = internalLookup(systemId);
  if ( index != INVALID_SME_INDEX ) throw SmeError();
  // ???????  что делать если уже в работе , шутдаунить прокси, как корректно или абортом
  records[index].info.disabled = false;*/
  __warning__("enableSme is not implemented");
}
#endif
// ----- SmeTable implementation ---------------------------

SmeIndex SmeManager::lookup(const SmeSystemId& systemId) const
{
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX ) throw runtime_error("can't find SME Proxy identifier");
  return index;
}

SmeProxy* SmeManager::getSmeProxy(SmeIndex index) const
{
__synchronized__
  SmeRecord* record = (SmeRecord*)(records.at(index));
  if ( record->deleted ) throw runtime_error("proxy deleted");
  if ( !record->proxy ) return 0;
  return (SmeProxy*)(record);
}

SmeInfo SmeManager::getSmeInfo(SmeIndex index) const
{
__synchronized__
  const SmeRecord* record = records.at(index);
  if ( record->deleted ) throw runtime_error("proxy deleted");
  return record->info;
}

void SmeManager::updateSmeInfo(const SmeSystemId& systemid,const SmeInfo& newinfo)
{
__synchronized__
  SmeIndex idx=internalLookup(systemid);
  if ( idx == INVALID_SME_INDEX ) throw runtime_error("can't find SME Proxy identifier");
  SmeRecord *r=records.at(idx);
  bool internal=r->info.internal;
  r->info=newinfo;
  r->info.internal=internal;
}


static uint32_t nextProxyUniqueId()
{
  static uint32_t unique = 0;
  return unique++;
}
// ------ SmeRegistrar implementation --------------------------

void SmeManager::registerInternallSmeProxy(const SmeSystemId& systemId,
                                SmeProxy* smeProxy)
{
__synchronized__

  __require__ ( smeProxy != NULL );
  {
    SmeProxyPriority p = smeProxy->getPriority();
    if (!( p > SmeProxyPriorityMinBr && p < SmeProxyPriorityMaxBr ))
      throw runtime_error("proxy has incorrect priority");
  }
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX )
  {
    throw runtime_error(string("unknown systm id:")+systemId);
  }
  if ( records[index]->proxy )
  {
    __trace2__("Failed to register proxy with sid:%s",systemId.c_str());
    __warning__("Sme proxy with tihs systemId already registered");
    throw runtime_error(string("proxy with id ")+systemId+" already exists");
  }
  {
    MutexGuard guard(records[index]->mutex);
    smeProxy->setPriority(records[index]->info.priority);
    records[index]->proxy = smeProxy;
    records[index]->uniqueId = nextProxyUniqueId();
    records[index]->info.internal=true;
  }
  dispatcher.attachSmeProxy(smeProxy,index);
}


SmeProxy* SmeManager::checkSmeProxy(const SmeSystemId& systemId,const SmePassword& pwd)
{
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX )
  {
    throw SmeRegisterException(SmeRegisterFailReasons::rfUnknownSystemId);
  }
  if ( records[index]->info.disabled)
  {
    __trace2__("Attempt to bind disabled sme:%s",systemId.c_str());
    throw SmeRegisterException(SmeRegisterFailReasons::rfDisabled);
  }
  if ( records[index]->info.password!=pwd)
  {
    __trace2__("Invalid password for sme %s (%s!=%s)",systemId.c_str(),
      records[index]->info.password.c_str(),pwd.c_str());
    throw SmeRegisterException(SmeRegisterFailReasons::rfInvalidPassword);
  }
  return records[index]->proxy;
}


void SmeManager::registerSmeProxy(const SmeSystemId& systemId,
                                  const SmePassword& pwd,
                                  SmeProxy* smeProxy)
{
__synchronized__

  __require__ ( smeProxy != NULL );
  {
    SmeProxyPriority p = smeProxy->getPriority();
    if (!( p > SmeProxyPriorityMinBr && p < SmeProxyPriorityMaxBr ))
      throw SmeRegisterException(SmeRegisterFailReasons::rfInternalError);
  }
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX )
  {
    throw SmeRegisterException(SmeRegisterFailReasons::rfUnknownSystemId);
  }
  if ( records[index]->info.disabled)
  {
    __trace2__("Attempt to bind disabled sme:%s",systemId.c_str());
    throw SmeRegisterException(SmeRegisterFailReasons::rfDisabled);
  }
  if ( records[index]->proxy )
  {
    __trace2__("Failed to register proxy with sid:%s",systemId.c_str());
    __warning__("Sme proxy with tihs systemId already registered");
    throw SmeRegisterException(SmeRegisterFailReasons::rfAlreadyRegistered);
  }
  if ( records[index]->info.password!=pwd)
  {
    __trace2__("Invalid password for sme %s (%s!=%s)",systemId.c_str(),
      records[index]->info.password.c_str(),
      pwd.c_str());
    smsc::util::Logger::getCategory("smeman.register").
      error("Attempt to register sme %s with invalid password",systemId.c_str());
    throw SmeRegisterException(SmeRegisterFailReasons::rfInvalidPassword);
  }
  {
    MutexGuard guard(records[index]->mutex);
    smeProxy->setPriority(records[index]->info.priority);
    records[index]->proxy = smeProxy;
    records[index]->uniqueId = nextProxyUniqueId();
    __trace2__("registerSme: smeId=%s, uniqueId=%d",systemId.c_str(),records[index]->uniqueId);
  }
  dispatcher.attachSmeProxy(smeProxy,index);
}

void SmeManager::unregisterSmeProxy(const SmeSystemId& systemId)
{

  SmeIndex index;
  {
    __synchronized__
    index = internalLookup(systemId);
    if ( index == INVALID_SME_INDEX ) throw runtime_error("is not registred");
  }

  {
    MutexGuard guard(records[index]->mutex);
    if ( records[index]->proxy )
      dispatcher.detachSmeProxy(records[index]->proxy);
    else
      __warning__("unregister null proxy");
    records[index]->proxy = 0;
  }
}

// SmeDispatcher implementation
SmeProxy* SmeManager::selectSmeProxy(unsigned long timeout,int* idx)
{
  int _idx;
  SmeProxy* proxy = dispatcher.dispatchIn(timeout,&_idx);
  if ( idx ) *idx = _idx;
  if ( proxy )
  {
  __synchronized__
    return (SmeProxy*)(records[_idx]);
  }
  else return 0;
}

SmeIndex SmeManager::internalLookup(const SmeSystemId& systemId) const
{
//__synchronized__ не нужно поскольку вызывается из синхронизированных методов
  for ( Records::const_iterator p = records.begin(); p != records.end(); ++p )
  {
    if ( (*p) == 0 )
    {
      __warning__("iterator pointed on null element");
    }
    else
    {
      if ( (*p)->deleted ) continue;
      if ( (*p)->info.systemId.compare(systemId) == 0 ) return (*p)->idx;
    }
  }
  return INVALID_SME_INDEX;
}

}; // namespace smeman
}; // namespace smsc
