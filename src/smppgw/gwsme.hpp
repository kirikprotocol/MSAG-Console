#ifndef __SMSC__SMPPGW_GWSME_HPP__
#define __SMSC__SMPPGW_GWSME_HPP__

#include "smeman/smeproxy.h"
#include "sme/SmppBase.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "smeman/smereg.h"
#include "util/Exception.hpp"

namespace smsc{
namespace smppgw{

using smsc::smeman::SmeProxy;
using smsc::sme::SmppSession;
using smsc::sme::SmeConfig;
using namespace smsc::core::threads;
using namespace smsc::smeman;
using namespace smsc::core::synchronization;

class GatewaySme:public SmeProxy,public ThreadedTask{
public:
  GatewaySme(const SmeConfig& cfg,smsc::smeman::SmeRegistrar* sr,const std::string& bh,int bp):lst(*this),sess(cfg,&lst),sesscfg(cfg),smereg(sr)
  {
    managerMonitor=NULL;
    connected=false;
    log=smsc::logger::Logger::getInstance("gwsme");
    seq=0;
    prefix=255;
    cfgIdx=0;
    hosts[0]=cfg.host;
    ports[0]=cfg.port;
    if(bh.length()>0)
    {
      hosts[1]=bh;
      ports[1]=bp;
    }else
    {
      hosts[1]=cfg.host;
      ports[1]=cfg.port;
    }
  }

  ~GatewaySme()
  {
    sess.close();
    smereg->unregisterSmeProxy(id);
  }

  int Execute();

  const char* taskName()
  {
    return "GatewaySme";
  }

  void close(){};
  /// ������ exception �᫨ �� ���⨣��� �����
  /// � ����� ��।� �� �� 㯠�� �� �����⨬��� ���祭��
  void putCommand(const SmscCommand& command)
  {
    MutexGuard g(mutexout);
    outqueue.Push(command);
    mutexout.notify();
  }


  bool getCommand(SmscCommand& cmd)
  {
    MutexGuard g(mutexin);
    if(inqueue.Count()==0)return false;
    inqueue.Pop(cmd);
    return true;
  }
  SmeProxyState getState()const
  {
    return smsc::smeman::VALID;
  }
  void init()
  {
  }

  bool hasInput()const
  {
    MutexGuard g(mutexin);
    return inqueue.Count()!=0;
  }

  void Enqueue(SmppHeader* pdu)
  {
    MutexGuard g(mutexin);
    inqueue.Push(SmscCommand(pdu));
    if(managerMonitor)managerMonitor->Signal();
  }

  void reportError(int errorcode)
  {
    if(errorcode==smsc::sme::smppErrorNetwork)
    {
      MutexGuard g(mutex);
      connected=false;
    }
  }

  bool isConnected()
  {
    MutexGuard g(mutex);
    return connected;
  }

  void attachMonitor(ProxyMonitor* monitor)
  {
    managerMonitor=monitor;
  }
  bool attached()
  {
    return managerMonitor!=NULL;
  }
  uint32_t getNextSequenceNumber()
  {
    MutexGuard g(mutex);
    return seq++;
  }

  void setId(const std::string& newid,SmeIndex idx)
  {
    __trace2__("setId:%s/%d",newid.c_str(),idx);
    id=newid;
    smeIndex=idx;
  }

  virtual SmeIndex getSmeIndex()
  {
    __trace2__("setIdx:%d",smeIndex);
    return smeIndex;
  }

  const char * getSystemId() const
  {
    return id.c_str();
  }


  void updateSmeInfo(const SmeInfo& _smeInfo){}

  // in and out MUST BE AT LEAST 32 BYTES!!!
  // return false if unfilled
  // port of peer is optional
  bool getPeers(char* in,char* out)
  {
    //TODO!!!
    strcpy(in,"127.0.0.1");
    strcpy(out,"127.0.0.1");
    return true;
  }

  void disconnect()
  {
  }

  void setPrefix(uint8_t p)
  {
    prefix=p;
  }

  uint8_t getPrefix()
  {
    if(prefix==255)throw smsc::util::Exception("prefix for %s not initialized",id.c_str());
    return prefix;
  }

protected:
  mutable Mutex mutex,mutexin;
  mutable EventMonitor mutexout;
  std::string id;
  SmeIndex smeIndex;
  int seq;
  smsc::core::buffers::CyclicQueue<SmscCommand> inqueue,outqueue;

  class PduListener:public smsc::sme::SmppPduEventListener{
  public:
    PduListener(GatewaySme& s):sme(s)
    {
      log=smsc::logger::Logger::getInstance("gwsme");
    }
    void handleEvent(SmppHeader *pdu)
    {
      debug2(log,"received pdu from smsc:%#X",pdu->get_commandId());
      try{
        sme.Enqueue(pdu);
      }catch(...)
      {
        warn2(log,"failed to construct command from pdu:%#X",pdu->get_commandId());
      }
      disposePdu(pdu);
      debug2(log,"enqueue of pdu %p ok",pdu);
    }
    void handleError(int errorCode)
    {
      sme.reportError(errorCode);
    }
  protected:
    GatewaySme& sme;
    smsc::logger::Logger* log;
  };
  PduListener lst;
  SmppSession sess;
  SmeConfig sesscfg;
  std::string hosts[2];
  int ports[2];
  int cfgIdx;
  smsc::smeman::SmeRegistrar* smereg;
  bool connected;
  smsc::logger::Logger* log;
  ProxyMonitor *managerMonitor;
  uint8_t prefix;
};


}//namespace smppgw
}//namespace smsc

#endif
