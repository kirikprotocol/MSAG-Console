/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_smeman_h__
#define __Cpp_Header__smeman_smeman_h__
#include <inttypes.h>
#include <vector>
#include <list>
#include <stdexcept>

#include "util/debug.h"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"

#include "smetypes.h"
#include "smeproxy.h"
#include "smeinfo.h"
#include "smeadmin.h"
#include "smetable.h"
#include "smeiter.h"
#include "smereg.h"


namespace smsc {
namespace smeman {

using std::runtime_error;
using std::string;


class SmeProxyWrap;

namespace SmeRegisterFailReasons{
  const int rfUnknownSystemId=0;
  const int rfAlreadyRegistered=1;
  const int rfInvalidPassword=2;
  const int rfInternalError=3;
  const int rfDisabled=4;
}

class SmeRegisterException:public std::exception{
public:
  SmeRegisterException(int reason):reason(reason){}
  int getReason()const{return reason;}
  const char* what()const throw()
  {
    switch(reason)
    {
      case SmeRegisterFailReasons::rfUnknownSystemId:return "Unknown systemId";
      case SmeRegisterFailReasons::rfAlreadyRegistered:return "Already registered";
      case SmeRegisterFailReasons::rfInvalidPassword:return "Invalid password";
      case SmeRegisterFailReasons::rfInternalError:return "Internal error";
      case SmeRegisterFailReasons::rfDisabled:return "Disabled";
    }
    return "Unknown error";
  }
protected:
  int reason;
};

class SmeRecord : public SmeProxy
{
public:
  uint32_t uniqueId;
  SmeInfo info;
  SmeProxy* proxy;
  bool deleted;
  SmeIndex idx;
  mutable Mutex mutex;

  Mutex refMtx;
  int refCnt;
  bool freeProxy;

  int seq;
  Mutex seqMtx;

  SmeRecord():seq(1),refCnt(0),deleted(false),freeProxy(false){}

  void acquire()
  {
    MutexGuard mg(refMtx);
    refCnt++;
  }

  void release()
  {
    MutexGuard mg(refMtx);
    refCnt--;
    if(freeProxy && refCnt==0)
    {
      if(proxy->deleteOnUnregister())delete proxy;
      proxy=0;
      freeProxy=false;
    }
  }

  struct RefGuard{
    RefGuard(SmeRecord* argRec):rec(argRec)
    {
      rec->acquire();
    }
    ~RefGuard()
    {
      rec->release();
    }
    SmeRecord* rec;
  };

  virtual ~SmeRecord(){}
  virtual void putCommand(const SmscCommand& command)
  {
    RefGuard rg(this);
    if ( proxy )
    {
      proxy->putCommand(command);
    }
    else throw runtime_error("proxy unregistred");
  }

  virtual int getCommandEx(std::vector<SmscCommand>& cmds,int& mx,SmeProxy* prx)
  {
    RefGuard rg(this);
    if ( proxy )
    {
      return proxy->getCommandEx(cmds,mx,prx);
    }
    else throw runtime_error("proxy unregistred");
  }

  virtual bool getCommand(SmscCommand& cmd)
  {
    RefGuard rg(this);
    if ( proxy )
    {
      return proxy->getCommand(cmd);
    }
    else throw runtime_error("proxy unregistred");
  }
  virtual uint32_t getNextSequenceNumber()
  {
    /*
    MutexGuard guard(mutex);
    if ( proxy )
    {
      return proxy->getNextSequenceNumber();
    }
    else throw runtime_error("proxy unregistred");
    */
    MutexGuard mg(seqMtx);
    if(seq==0)seq++;
    return seq++;
  }
  virtual uint32_t getUniqueId() const {return uniqueId;}
  virtual bool hasInput() const {__unreachable__("");return 0;}
  virtual SmeProxyState getState() const {__unreachable__("");return INVALID;}
  virtual void init(){__unreachable__("");}
  virtual SmeProxyPriority getPriority() const
  {
//    MutexGuard guard(mutex);
    return info.priority;
    /*
    if ( proxy )
    {
      return //proxy->getPriority();
    }
    else throw runtime_error("proxy unregistred");
    */
  }
  virtual void attachMonitor(ProxyMonitor* monitor) {__unreachable__("");}
  virtual bool attached(){__unreachable__("");return 0;}
  virtual void close() {__unreachable__("");}
  virtual void disconnect()
  {
    RefGuard rg(this);
    if ( proxy )
    {
      proxy->disconnect();
    }
    else throw runtime_error("proxy not connected");
  }
  virtual uint32_t getIndex()const{ return idx;}
  virtual unsigned long getPreferredTimeout() {
    return info.timeout;
  }
  virtual const std::string& getSourceAddressRange()
  {
    return info.rangeOfAddress;
  }
  virtual const char* getSystemId()const
  {
    return info.systemId.c_str();
  }
  virtual int getProviderId() const
  {
    return info.providerId;
  };
  virtual uint32_t getAccessMask()const
  {
    return info.accessMask;
  }
  virtual SmeIndex getSmeIndex()
  {
    return idx;
  }
  virtual int getBindMode()
  {
    MutexGuard guard(mutex);
    if ( proxy )
    {
      return proxy->getBindMode();
    }
    else throw runtime_error("proxy not connected");
  }
  virtual bool getPeers(char* in,char* out)
  {
    MutexGuard guard(mutex);
    if ( proxy )
    {
      return proxy->getPeers(in,out);
    }
    else throw runtime_error("proxy not connected");
  }
};

using core::synchronization::Mutex;
typedef std::vector<SmeRecord*> Records;

class SmeManager :
  public SmeAdministrator,
  public SmeTable,
  public SmeRegistrar
{

  mutable Mutex lock;
  ProxyMonitor mon;
  Records records;
  SmeIndex internalLookup(const SmeSystemId& systemId) const;
public:
  SmeManager()
  {
    records.reserve(MAX_SME_PROXIES);
  }
  virtual ~SmeManager()
  {
    Records::iterator it = records.begin();
    for ( ; it != records.end(); ++it )
    {
      try
      {
        if ( (*it)->proxy )
        {
          __warning2__("proxy with system id %s is attached when destroy smeman",(*it)->info.systemId.c_str());
          //dispatcher.detachSmeProxy((*it)->proxy);
        }
        delete (*it);
      }
      catch(...)
      {
        __warning__("error when detach proxy");
      }
    }
  }
  //....
  // SmeAdministrator implementation
  virtual void addSme(const SmeInfo& info);
  virtual void deleteSme(const SmeSystemId& systemId);
//  virtual void store();
  virtual SmeIterator* iterator(); // client must destroy returned object
#if 0
  virtual void disableSme(const SmeSystemId& systemId);
  virtual void enableSme(const SmeSystemId& systemId);
#endif
  // SmeTable implementation
  virtual SmeIndex lookup(const SmeSystemId& systemId) const;
  virtual SmeProxy* getSmeProxy(SmeIndex index) const;
  virtual SmeInfo getSmeInfo(SmeIndex index) const;

  virtual void updateSmeInfo(const SmeSystemId& systemid,const SmeInfo& newinfo);

/*
  // SmeIterator implementation
  virtual bool next();
  virtual SmeProxy* getSmeProxy() const;
  virtual SmeInfo  getSmeInfo() const;
  virtual SmeIndex getSmeIndex() const; // ?????
*/
  // SmeRegistrar implementation
  virtual void registerSmeProxy(const SmeSystemId& systemId,
                                const SmePassword& pwd,
                                SmeProxy* smeProxy);
  virtual void registerInternallSmeProxy(const SmeSystemId& systemId,
                                SmeProxy* smeProxy);

  SmeProxy* checkSmeProxy(const SmeSystemId& systemId,const SmePassword& pwd);
  virtual void unregisterSmeProxy(SmeProxy* smeProxy);

  // SmeDispatcher implementation
  //virtual SmeProxy* selectSmeProxy(unsigned long timeout=0,int* idx=0);
  virtual void getFrame(vector<SmscCommand>& frames, unsigned long timeout,bool skipScheduler);
  //virtual ~SmeManager(){}

  void unregSmsc(const SmeSystemId& systemId);

  void Dump()
  {
    MutexGuard mg(lock);
    Records::iterator it = records.begin();
    for ( ; it != records.end(); ++it )
    {
      __warning2__("Sme %s %s",(*it)->info.systemId.c_str(),(*it)->proxy?"registered":"unregistered");
    }
  }

  size_t getSmesCount()
  {
    MutexGuard mg(lock);
    return records.size();
  }

};

} // namespace smeman
} // namespace smsc
#endif
