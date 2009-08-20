/*
  $Id$
*/

#include <exception>
#include "util/debug.h"
#include "smsccmd.h"
#include "core/synchronization/Event.hpp"
#include <vector>
#include "util/sleep.h"

#if !defined __Cpp_Header__smeman_smeproxy_h__
#define __Cpp_Header__smeman_smeproxy_h__

namespace smsc {
namespace smeman {

enum SmeProxyState
{
  VALID,
  INVALID
};

enum SmeBindMode{
  smeRX,smeTX,smeTRX
};


typedef int SmeProxyPriority;
const int SmeProxyPriorityMinBr = 0;
const int SmeProxyPriorityMin = 1;
const int SmeProxyPriorityDefault = 16000;
const int SmeProxyPriorityMax = 32000;
const int SmeProxyPriorityMaxBr = SmeProxyPriorityMax+1;

struct PolledMonitor
{
  volatile bool state;
  PolledMonitor():state(false){}
  void Signal()
  {
    state=true;
  }
  void Wait(int to)
  {
    while(!state && to>0)
    {
      smsc::util::millisleep(10);
      to-=10;
    }
    state=false;
  }
};

//typedef smsc::core::synchronization::Event ProxyMonitor;
typedef PolledMonitor ProxyMonitor;

struct SmeInfo;

class ProxyLimitException : public std::exception
{
protected:
  char msg[32];
public:
  ProxyLimitException(const char* extype)
  {
    sprintf(msg,"%.31s",extype);
  }
  ProxyLimitException(const char* extype,int cur,int mx)
  {
    sprintf(msg,"%.11s:%d/%d",extype,cur,mx);
  }
  virtual const char* what() const throw()
  {
    return msg;
  }
  ~ProxyLimitException()throw(){}
};


class ProxyQueueLimitException : public ProxyLimitException
{
public:
  ProxyQueueLimitException():ProxyLimitException("PXQLimit")
  {
  }
  ProxyQueueLimitException(int c,int m):ProxyLimitException("PXQLimit",c,m)
  {
  }
  ~ProxyQueueLimitException()throw(){}
};

class ProxyShapeLimitException:public ProxyLimitException
{
public:
  ProxyShapeLimitException():ProxyLimitException("PXSLimit")
  {
  }
  ProxyShapeLimitException(int c,int m):ProxyLimitException("PXSLimit",c,m)
  {
  }
  ~ProxyShapeLimitException()throw(){}
};

class InvalidProxyCommandException: public std::exception
{
public:
  virtual const char* what() const throw() {return "Invalid bind state";}
  ~InvalidProxyCommandException()throw(){}
};

// abstract
class SmeProxy
{
public:
  SmeProxy():nullstr(""){}
  //....
  virtual ~SmeProxy() {}
  virtual void close() = 0;
  ///  exception    
  ///         
  virtual void putCommand(const SmscCommand& command) = 0;
  virtual bool getCommand(SmscCommand& cmd) = 0;

  //     cmds   mx .
  //  mx      .
  //   -   .
  // prx -    SmeRecord,  
  //  .
  virtual int getCommandEx(std::vector<SmscCommand>& cmds,int& mx,SmeProxy* prx)
  {
    SmscCommand cmd;
    if(getCommand(cmd))
    {
      cmd.setProxy(prx);
      cmds.push_back(cmd);
      mx=1;
    }else
    {
      mx=0;
    }
    return 0;
  }
  virtual SmeProxyState getState() const = 0;
  virtual void init() = 0;
  virtual SmeProxyPriority getPriority()const {return SmeProxyPriorityDefault;};
  virtual void setPriority(SmeProxyPriority){};
  virtual bool hasInput() const = 0;
  virtual void attachMonitor(ProxyMonitor* monitor) = 0; // for detach monitor call with NULL
  virtual bool attached() = 0; // check what proxy has attached monitor (proxy is attached to dispatcher)
  virtual uint32_t getNextSequenceNumber() = 0;
  virtual uint32_t getUniqueId() const {__unreachable__("must be implemented in proxy or wrapper");return 0;}
  virtual uint32_t getIndex() const {__unreachable__("must be implemented in proxy or wrapper");return 0;}
  virtual unsigned long getPreferredTimeout() { return 8; }
  virtual const std::string& getSourceAddressRange(){return nullstr;};
  virtual const char * getSystemId() const = 0;
  virtual int getProviderId() const {return -1;};
  virtual uint32_t getAccessMask()const{return 0;}

  virtual void updateSmeInfo(const SmeInfo& _smeInfo){}

  virtual SmeIndex getSmeIndex(){return INVALID_SME_INDEX;}

  virtual int getBindMode(){return smeTRX;}

  // in and out MUST BE AT LEAST 32 BYTES!!!
  // return false if unfilled
  // port of peer is optional
  virtual bool getPeers(char* in,char* out)
  {
    strcpy(in,"127.0.0.1");
    strcpy(out,"127.0.0.1");
    return true;
  };

  virtual void disconnect(){};
  virtual bool deleteOnUnregister(){return false;}

  std::string nullstr;
};

} // namespace smeman
} // namespace smsc

#endif
