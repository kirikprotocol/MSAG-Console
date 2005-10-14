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
#include "core/buffers/RefPtr.hpp"
#include "router/route_manager.h"
#include "SmppRouter.h"
#include "core/threads/ThreadPool.hpp"

namespace scag{
namespace transport{
namespace smpp{

namespace buf=smsc::core::buffers;
namespace thr=smsc::core::threads;
namespace sync=smsc::core::synchronization;


class SmppManager:
  public SmppManagerAdmin,
  public SmppChannelRegistrator,
  public SmppCommandQueue,
  public SmppRouter{
public:
  SmppManager();
  ~SmppManager();
  void Init(const char* cfgFile);
  void LoadRoutes(const char* cfgFile);
  void ReloadRoutes();

  //admin
  virtual void addSmppEntity(const SmppEntityInfo& info);
  virtual void updateSmppEntity(const SmppEntityInfo& info);
  virtual void deleteSmppEntity(const char* sysId);

  //registragor
  virtual int registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch);
  virtual int registerSmscChannel(SmppChannel* ch);
  virtual void unregisterChannel(SmppChannel* ch);

  //queue
  virtual void putCommand(SmppChannel* ct,SmppCommand& cmd);
  virtual bool getCommand(SmppCommand& cmd);

  void StopProcessing()
  {
    sync::MutexGuard mg(queueMon);
    running=false;
    queueMon.notifyAll();
  }

  //SmppRouter
  virtual SmppEntity* RouteSms(router::SmeIndex srcidx, const smsc::sms::Address& source, const smsc::sms::Address& dest, router::RouteInfo& info)
  {
    {
      RouterRef ref=routeMan;
      if(!ref->lookup(srcidx,source,dest,info))return 0;
    }
    MutexGuard mg(regMtx);
    SmppEntity** ptr=registry.GetPtr(info.smeSystemId);
    if(!ptr)return 0;
    return *ptr;
  }

protected:
  smsc::logger::Logger* log;
  buf::Hash<SmppEntity*> registry;
  sync::Mutex regMtx;
  SmppSocketManager sm;

  bool running;

  buf::CyclicQueue<SmppCommand> queue;
  sync::EventMonitor queueMon;

  typedef RefPtr<router::RouteManager,sync::Mutex> RouterRef;
  RouterRef routeMan;
  std::string routerConfigFile;

  thr::ThreadPool tp;

  int lastUid;
};

}//smpp
}//transport
}//scag


#endif
