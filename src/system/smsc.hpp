#ifndef __SYSTEM_SMSC_HPP__
#define __SYSTEM_SMSC_HPP__

#include "core/threads/ThreadPool.hpp"
#include "util/config/Manager.h"
#include "system/smppio/SmppSocketsManager.hpp"
#include "smeman/smeman.h"
#include "task_container.h"
#include "router/route_manager.h"

namespace smsc{
namespace system{

using smsc::sms::SMS;
using smsc::smeman::SmeProxy;
using smsc::router::RouteManager;
using smsc::router::RouteInfo;

class Smsc
{
public:
  Smsc():ssockman(&tp,&smeman){};
  ~Smsc();
  void init();
  void run();
  void mainLoop();
  TaskContainer tasks;  
	bool Smsc::routeSms(SMS* sms, int& dest_idx,SmeProxy*& proxy);
protected:
  smsc::core::threads::ThreadPool tp;
  smsc::util::config::Manager* cfgman;
  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
	smsc::router::RouteManager router;
};

};//system
};//smsc


#endif
