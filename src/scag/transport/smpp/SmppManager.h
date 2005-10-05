#ifndef __SCAG_TRANSPORT_SMPP_SMPPMANAGER_H__
#define __SCAG_TRANSPORT_SMPP_SMPPMANAGER_H__

#include "SmppManagerAdmin.h"
#include "logger/Logger.h"
#include "core/buffers/Hash.hpp"
#include "SmppEntity.h"
#include "SmppChannelRegistrator.h"
#include "SmppCommandQueue.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "SmppSocketManager.h"
#include "core/buffers/CyclicQueue.hpp"

namespace scag{
namespace transport{
namespace smpp{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;


class SmppManager:public SmppManagerAdmin,SmppChannelRegistrator,SmppCommandQueue{
public:
  SmppManager();
  ~SmppManager();
  void Init(const char* cfgFile);

  //admin
  virtual void addSmppEntity(const SmppEntityInfo& info);
  virtual void updateSmppEntity(const SmppEntityInfo& info);
  virtual void deleteSmppEntity(const char* sysId);

  //registragor
  virtual int registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch);
  virtual int registerSmscChannel(SmppChannel* ch);
  virtual void unregisterChannel(SmppChannel* ch);

  //queue
  virtual void putCommand(SmppBindType ct,const SmppCommand& cmd);
  virtual bool getCommand(SmppCommand& cmd);

  void StopProcessing()
  {
    sync::MutexGuard mg(queueMon);
    running=false;
    queueMon.notifyAll();
  }

protected:
  smsc::logger::Logger* log;
  buf::Hash<SmppEntity*> registry;
  sync::Mutex regMtx;
  SmppSocketManager sm;

  bool running;

  buf::CyclicQueue<SmppCommand> queue;
  sync::EventMonitor queueMon;
};

}//smpp
}//transport
}//scag


#endif
