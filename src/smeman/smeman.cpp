/*
  $Id$
*/
#include "smeman.h"
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
  SmeRecord record;
  record.info = info;
  record.proxy = 0;
  record.deleted = false;
  record.idx = records.size();
  SmeIndex index = internalLookup(info.systemId);
  if ( index != INVALID_SME_INDEX ) throw SmeError();
  records.push_back(record);
}

void SmeManager::deleteSme(const SmeSystemId& systemId)
{
__synchronized__
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX ) throw SmeError();
  if ( records[index].proxy )
  {
    // ???????  что делать если уже в работе , шутдаунить прокси, как корректно или абортом
    // к примеру так, но тогда , что делает close?
    dispatcher.detachSmeProxy(records[index].proxy);
    records[index].proxy->close();
    //delete records[index].proxy;
    records[index].proxy = 0;
  }
  records[index].deleted = true;
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
    if (started&&ptr!=end) ++ptr;
    started = true;
    return ptr != end;
  }

  virtual SmeProxy* getSmeProxy() const
  {
    return (*ptr).proxy;
  }
  virtual SmeInfo  getSmeInfo() const
  {
    return (*ptr).info;
  }
  virtual SmeIndex getSmeIndex() const
  {
    return (*ptr).idx;
  }
};

SmeIterator* SmeManager::iterator()
{
__synchronized__
  return new SmeIteratorImpl(records.begin(),records.end());
}

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

// ----- SmeTable implementation ---------------------------

SmeIndex SmeManager::lookup(const SmeSystemId& systemId) const
{
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX ) throw SmeError();
  return index;
}

SmeProxy* SmeManager::getSmeProxy(SmeIndex index) const
{
__synchronized__
  const SmeRecord& record = records.at(index);
  if ( record.deleted ) throw SmeError();
  return record.proxy;
}

SmeInfo SmeManager::getSmeInfo(SmeIndex index) const
{
__synchronized__
  const SmeRecord& record = records.at(index);
  if ( record.deleted ) throw SmeError();
  return record.info;
}

// ------ SmeRegistrar implementation --------------------------

void SmeManager::registerSmeProxy(const SmeSystemId& systemId, SmeProxy* smeProxy)
{
__synchronized__

  __require__ ( smeProxy != NULL );

  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX )
  {
    throw runtime_error(string("unknown systm id:")+systemId);
  }
  if ( records[index].proxy )
  {
    __trace2__("Failed to register proxy with sid:%s",systemId.c_str());
    __warning__("Sme proxy with tihs systemId already registered");
    throw runtime_error(string("proxy with id ")+systemId+" already exists");
  }
  records[index].proxy = smeProxy;
  dispatcher.attachSmeProxy(smeProxy,index);
}

void SmeManager::unregisterSmeProxy(const SmeSystemId& systemId)
{
__synchronized__

  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX ) throw SmeError();
  if ( records[index].proxy )
    dispatcher.detachSmeProxy(records[index].proxy);
  else
    __warning__("unregister null proxy");
  records[index].proxy = 0;
}

// SmeDispatcher implementation
SmeProxy* SmeManager::selectSmeProxy(unsigned long timeout,int* idx)
{
  return dispatcher.dispatchIn(timeout,idx);
}

SmeIndex SmeManager::internalLookup(const SmeSystemId& systemId) const
{
//__synchronized__ не нужно поскольку вызывается из синхронизированных методов
  for ( Records::const_iterator p = records.begin(); p != records.end(); ++p )
  {
    if ( p->info.systemId.compare(systemId) == 0 ) return p->idx;
  }
  return INVALID_SME_INDEX;
}

}; // namespace smeman
}; // namespace smsc
