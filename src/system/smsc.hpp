#ifndef __SYSTEM_SMSC_HPP__
#define __SYSTEM_SMSC_HPP__

#include "core/threads/ThreadPool.hpp"
#include "util/config/Manager.h"
#include "system/smppio/SmppSocketsManager.hpp"
#include "smeman/smeman.h"
#include "task_container.h"
#include "router/route_manager.h"
#include "system/event_queue.h"
#include "store/MessageStore.h"

namespace smsc{
namespace system{

using smsc::sms::SMS;
using smsc::smeman::SmeProxy;
using smsc::router::RouteManager;
using smsc::router::RouteInfo;

//class smsc::store::MessageStore;

class Smsc
{
public:
  Smsc():ssockman(&tp,&smeman),stopFlag(false){};
  ~Smsc();
  void init();
  void run();
  void stop(){stopFlag=true;}
  void mainLoop();
  void shutdown();
  TaskContainer tasks;
  bool Smsc::routeSms(SMS* sms, int& dest_idx,SmeProxy*& proxy);
protected:
  smsc::core::threads::ThreadPool tp;
  smsc::util::config::Manager* cfgman;
  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
  smsc::router::RouteManager router;
  EventQueue eventqueue;
  smsc::store::MessageStore *store;
  bool stopFlag;
};

};//system
};//smsc


#endif
