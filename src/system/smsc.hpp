#ifndef __SYSTEM_SMSC_HPP__
#define __SYSTEM_SMSC_HPP__

#include "core/threads/ThreadPool.hpp"
#include "util/config/Manager.h"
#include "system/smppio/SmppSocketsManager.hpp"
#include "smeman/smeman.h"
#include "task_container.h"

namespace smsc{
namespace system{
using smsc::sms::SMS;
using smsc::smeman::SmeProxy;

class Smsc
{
public:
  Smsc():ssockman(&tp,&smeman){};
  ~Smsc();
  void init();
  void run();
  void mainLoop();
  TaskContainer tasks;  
  SmeProxy* routeSms(SMS* sms, int* idex =0);
protected:
  smsc::core::threads::ThreadPool tp;
  smsc::util::config::Manager* cfgman;
  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
};

};//system
};//smsc


#endif
