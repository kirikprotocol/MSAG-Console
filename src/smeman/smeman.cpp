/*
  $Id$
*/
#include "smeman.h"
#include "logger/Logger.h"
#include <stdexcept>
#include <string>
#include "admin/service/Variant.h"
#include "admin/service/Type.h"
#include "smppgw/gwsme.hpp"

#include "logger/Logger.h"

namespace smsc {
namespace smeman {

using namespace std;
using namespace smsc::admin::service;
using smsc::smppgw::GatewaySme;
using namespace smsc::logger;

using core::synchronization::MutexGuard;
#define __synchronized__ MutexGuard mguard(lock);

// ---- SmeAdministrator implementation --------------

void SmeManager::addSme(const SmeInfo& info)
{
__synchronized__
  if(records.size()==MAX_SME_PROXIES)throw runtime_error("Maximum number of sme's reached");
  auto_ptr<SmeRecord> record(new SmeRecord);
  record->info = info;
  record->info.internal=false;
  record->proxy = 0;
  record->deleted = false;
  record->idx = (int)records.size();
  SmeIndex index = internalLookup(info.systemId);
  if ( index != INVALID_SME_INDEX ) throw runtime_error("Already exists");
  records.push_back(record.release());
}

void SmeManager::statusSme(Variant& result){
    MutexGuard mg(lock);
    Records::iterator it = records.begin();

    for ( ; it != records.end(); ++it )
    {
        std::string status;
        status += (*it)->info.systemId;
        status += ",";


        (*it)->mutex.Lock();
        bool rv=(*it)->proxy!=NULL;
        // if gateway check isConnected method.
        if ((*it)->info.internal){
            rv = ((smsc::smppgw::GatewaySme*)((*it)->proxy))->isConnected();
        }
        (*it)->mutex.Unlock();

        if (!rv)
        {
            status += "disconnected";
        }
        else
        {
            SmeProxy * smeProxy = (*it)->proxy;
            try {
               std::string tmpStr;
               switch (smeProxy->getBindMode())
               {
               case smeTX:
                    tmpStr += "tx,";
                    break;
               case smeRX:
                    tmpStr += "rx,";
                    break;
               case smeTRX:
                    tmpStr += "trx,";
                    break;
               default:
                    tmpStr += "unknown,";
               }
               char inIP[128], outIP[128];
               for(int i=0; i<=127; i++)
                   inIP[i] = 0;
               if (smeProxy->getPeers(inIP,outIP))
               {
                   tmpStr += inIP;
                   tmpStr += ",";
                   tmpStr += outIP;
               }
               else
               {
                   tmpStr += "unknown,unknown";
               }
               status += tmpStr;
            } catch (...) {
               status += "unknown,unknown,unknown";
            }

        }
        result.appendValueToStringList(status.c_str());
    }
  }

void SmeManager::unregSmsc(const SmeSystemId& systemId)
{
  __synchronized__
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX ) throw SmeError();

  records[index]->uniqueId = 0;
  records[index]->info.internal = false;
}

void SmeManager::deleteSme(const SmeSystemId& systemId)
{
__synchronized__
  SmeIndex index = internalLookup(systemId);
  if ( index == INVALID_SME_INDEX ) throw SmeError();
  if ( records[index]->proxy )
  {
    // ???????        ,  ,
    //   ,   ,   close?
    //dispatcher.detachSmeProxy(records[index]->proxy);
    records[index]->proxy->attachMonitor(0);
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
    return (*ptr);
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
  virtual bool isSmeConnected()const
  {
    if ( ptr == end ) throw runtime_error("out of data");
    (*ptr)->mutex.Lock();
    bool rv=(*ptr)->proxy!=NULL;
    (*ptr)->mutex.Unlock();
    return rv;
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
  // ???????        ,  ,
  records[index].info.disabled = true;*/
  __warning__("disableSme is not implemented");
}

void SmeManager::enableSme(const SmeSystemId& systemId)
{
  /*SmeIndex index = internalLookup(systemId);
  if ( index != INVALID_SME_INDEX ) throw SmeError();
  // ???????        ,  ,
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
//__synchronized__
  SmeRecord* record = (SmeRecord*)(records.at(index));
  if ( record->deleted ) throw runtime_error("proxy deleted");
  if ( !record->proxy ) return 0;
  return (SmeProxy*)(record);
}

SmeInfo SmeManager::getSmeInfo(SmeIndex index) const
{
//__synchronized__
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
  {
    MutexGuard mg(r->mutex);
    if(r->proxy)
    {
      r->proxy->updateSmeInfo(r->info);
    }
  }
}


static uint32_t nextProxyUniqueId()
{
  static uint32_t unique = 0;
  return ++unique;
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
    throw runtime_error(string("unknown systm id:")+systemId.c_str());
  }
  if ( records[index]->proxy )
  {
    __trace2__("Failed to register proxy with sid:%s",systemId.c_str());
    __warning__("Sme proxy with this systemId already registered");
    throw runtime_error(string("proxy with id ")+systemId.c_str()+" already exists");
  }
  {
    MutexGuard guard(records[index]->mutex);
    smeProxy->setPriority(records[index]->info.priority);
    records[index]->acquire();
    records[index]->proxy = smeProxy;
    records[index]->uniqueId = nextProxyUniqueId();
    records[index]->info.internal=true;
  }
  smeProxy->attachMonitor(&mon);
  //dispatcher.attachSmeProxy(smeProxy,index);
}


SmeProxy* SmeManager::checkSmeProxy(const SmeSystemId& systemId,const SmePassword& pwd)
{
__synchronized__
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
    __warning2__("Invalid password for sme %s (%s!=%s)",systemId.c_str(),
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
  if ( records[index]->info.password!=pwd)
  {
    __trace2__("Invalid password for sme %s (%s!=%s)",systemId.c_str(),
      records[index]->info.password.c_str(),
      pwd.c_str());
      smsc_log_error(smsc::logger::Logger::getInstance("smeman.register"), "Attempt to register sme %s with invalid password",systemId.c_str());
    throw SmeRegisterException(SmeRegisterFailReasons::rfInvalidPassword);
  }

  {
    MutexGuard guard(records[index]->mutex);

    if ( records[index]->proxy )
    {
      __trace2__("Failed to register proxy with sid:%s",systemId.c_str());
      __warning__("Sme proxy with tihs systemId already registered");
      throw SmeRegisterException(SmeRegisterFailReasons::rfAlreadyRegistered);
    }

    smeProxy->setPriority(records[index]->info.priority);
    records[index]->acquire();
    records[index]->proxy = smeProxy;
    records[index]->uniqueId = nextProxyUniqueId();
    __trace2__("registerSme: smeId=%s, uniqueId=%d",systemId.c_str(),records[index]->uniqueId);
  }
  //dispatcher.attachSmeProxy(smeProxy,index);
  smeProxy->attachMonitor(&mon);
}

void SmeManager::unregisterSmeProxy(SmeProxy* smeProxy)
{
  SmeIndex index;
  __synchronized__
  index = internalLookup(smeProxy->getSystemId());
  if ( index == INVALID_SME_INDEX ) throw runtime_error("sme proxy is not registred");

  MutexGuard guard(records[index]->mutex);
  if ( records[index]->proxy )
  {
    if(records[index]->proxy==smeProxy)
    {
      records[index]->proxy->attachMonitor(0);
      records[index]->freeProxy=true;
      records[index]->release();
    }else
    {
      __warning2__("Attempt to unregister incorrect proxy:%s",smeProxy->getSystemId());
    }
  }
  else
  {
    __warning2__("unregister null proxy(%s)",smeProxy->getSystemId());
  }
}

// SmeDispatcher implementation
/*
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
*/

// SmeDispatcher implementation
void SmeManager::getFrame(vector<SmscCommand>& frames, unsigned long timeout,bool skipScheduler)
{
  static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("smeman");
  {
    frames.clear();
    //__synchronized__
    int count=0;
    Records::const_iterator end=records.end();
    for ( Records::const_iterator p = records.begin(); p != end; ++p )
    {
      if ( (*p) )
      {
        if ((*p)->proxy==NULL || (*p)->deleted) continue;
        if(skipScheduler && (*p)->info.systemId=="scheduler")continue;

        int prio=(*p)->info.priority/1000;
        if(prio<0)prio=0;
        if(prio>31)prio=31;
        prio+=1;


        try {
          int cnt=0;
          {
            //MutexGuard mg((*p)->mutex);
            SmeRecord::RefGuard rg(*p);
            if((*p)->proxy)
            {
              cnt=(*p)->proxy->getCommandEx(frames,prio,*p);
            }
          }
          count+=cnt;
        }catch(exception& e)
        {
          __warning2__("exception %s when getting command",e.what());
        }

      }
    }
    if(count)info2(log,"total commands in queues:%d",count);
  }
  if ( !frames.size() ) mon.Wait((int)timeout);
}

SmeIndex SmeManager::internalLookup(const SmeSystemId& systemId) const
{
//__synchronized__
  for ( Records::const_iterator p = records.begin(); p != records.end(); ++p )
  {
    if ( (*p) == 0 )
    {
      __warning__("iterator pointed on null element");
    }
    else
    {
      if ( (*p)->deleted ) continue;
      if ( (*p)->info.systemId==systemId ) return (*p)->idx;
    }
  }
  return INVALID_SME_INDEX;
}

} // namespace smeman
} // namespace smsc
