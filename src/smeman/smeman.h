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
#include "smetypes.h"
#include "smeproxy.h"
#include "smeinfo.h"
#include "smeadmin.h"
#include "smetable.h"
#include "smeiter.h"
#include "smereg.h"
#include "smedispatch.h"
#include "dispatch.h"
#include "core/synchronization/Mutex.hpp"

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
};

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
      case SmeRegisterFailReasons::rfInvalidPassword:return "Invalid passowrd";
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
  virtual ~SmeRecord(){}
  virtual void putCommand(const SmscCommand& command)
  {
    MutexGuard guard(mutex);
    if ( proxy )
    {
      proxy->putCommand(command);
    }
    else throw runtime_error("proxy unregistred");
  }
  virtual SmscCommand getCommand()
  {
    MutexGuard guard(mutex);
    if ( proxy )
    {
      return proxy->getCommand();
    }
    else throw runtime_error("proxy unregistred");
  }
  virtual uint32_t getNextSequenceNumber()
  {
    MutexGuard guard(mutex);
    if ( proxy )
    {
      return proxy->getNextSequenceNumber();
    }
    else throw runtime_error("proxy unregistred");
  }
  virtual uint32_t getUniqueId() const {return uniqueId;}
  virtual bool hasInput() const {__unreachable__("");return 0;}
  virtual SmeProxyState getState() const {__unreachable__("");return INVALID;}
  virtual void init(){__unreachable__("");}
  virtual SmeProxyPriority getPriority() const {__unreachable__("");return 0;}
  virtual void attachMonitor(ProxyMonitor* monitor) {__unreachable__("");}
  virtual bool attached(){__unreachable__("");return 0;}
  virtual void close() {__unreachable__("");}
  virtual void disconnect()
  {
    MutexGuard guard(mutex);
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
  virtual SmeIndex getSmeIndex()
  {
    return idx;
  }
};

using core::synchronization::Mutex;
typedef std::vector<SmeRecord*> Records;

class SmeManager :
  public SmeAdministrator,
  public SmeTable,
  public SmeRegistrar,
  public Dispatch
{

  mutable Mutex lock;
  SmeProxyDispatcher dispatcher;
  Records records;
  SmeIndex internalLookup(const SmeSystemId& systemId) const;
public:
  virtual ~SmeManager()
  {
    Records::iterator it = records.begin();
    for ( ; it != records.end(); ++it )
    {
      try
      {
        if ( (*it)->proxy )
        {
          __warning__((string("proxy with system id ")+
                      (*it)->info.systemId+
                      string(" is attached when destroy smeman")).c_str());
          dispatcher.detachSmeProxy((*it)->proxy);
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
  virtual void unregisterSmeProxy(const SmeSystemId& systemId);

  // SmeDispatcher implementation
  virtual SmeProxy* selectSmeProxy(unsigned long timeout=0,int* idx=0);
  virtual void getFrame(vector<SmscCommand>& frames, unsigned long timeout);
  //virtual ~SmeManager(){}
};

}; // namespace smeman
}; // namespace smsc
#endif
