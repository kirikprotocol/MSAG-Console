#ifndef __SMSC_NULLSME_HPP__
#define __SMSC_NULLSME_HPP__

#include "core/threads/ThreadedTask.hpp"
#include "smeman/smeproxy.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/Mutex.hpp"


namespace smsc{

class NullSme:public smsc::smeman::SmeProxy{
protected:
  smsc::core::buffers::CyclicQueue<smsc::smeman::SmscCommand> queue;
  smsc::core::synchronization::Mutex mtx;
  smsc::smeman::ProxyMonitor* monitor;
  std::string sysId;
  uint32_t seqNum;
  smsc::core::synchronization::Mutex seqMtx;
public:
  NullSme()
  {
    monitor=0;
    seqNum=0;
    sysId="NULLSME";
  }
  virtual void close()
  {

  }
  virtual void putCommand(const smsc::smeman::SmscCommand& command)
  {
    if(command->cmdid==smsc::smeman::DELIVERY)
    {
      smsc::smeman::SmscCommand resp=smsc::smeman::SmscCommand::makeDeliverySmResp("",command->get_dialogId(),0);
      smsc::core::synchronization::MutexGuard mg(mtx);
      queue.Push(resp);
      if(monitor)
      {
        monitor->Signal();
      }
    }
  }
  virtual bool getCommand(smsc::smeman::SmscCommand& cmd)
  {
    smsc::core::synchronization::MutexGuard mg(mtx);
    if(queue.Count())
    {
      queue.Pop(cmd);
      return true;
    }
    return false;
  }

  virtual smsc::smeman::SmeProxyState getState() const
  {
    return smsc::smeman::VALID;
  }
  virtual void init()
  {

  }
  virtual smsc::smeman::SmeProxyPriority getPriority()const {return smsc::smeman::SmeProxyPriorityDefault;};
  virtual void setPriority(smsc::smeman::SmeProxyPriority){};
  virtual bool hasInput() const
  {
    return queue.Count()!=0;
  }
  virtual void attachMonitor(smsc::smeman::ProxyMonitor* argMonitor)
  {
    monitor=argMonitor;
  }
  virtual bool attached()
  {
    return monitor!=0;
  }
  virtual uint32_t getNextSequenceNumber()
  {
    smsc::core::synchronization::MutexGuard mg(seqMtx);
    return seqNum++;
  }
  virtual const char * getSystemId() const
  {
    return sysId.c_str();
  }

  virtual void updateSmeInfo(const smsc::smeman::SmeInfo& _smeInfo)
  {

  }

};

}

#endif //__SMSC_SYSTEM_NULLSME_HPP__
