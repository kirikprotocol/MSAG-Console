#ifndef __SYSTEM_SMSC_HPP__
#define __SYSTEM_SMSC_HPP__

#include "core/threads/ThreadPool.hpp"
#include "util/config/Manager.h"
#include "system/smppio/SmppSocketsManager.hpp"
#include "smeman/smeman.h"

namespace smsc{
namespace system{

class Smsc{
public:
  Smsc():ssockman(&tp,&smeman){};
  ~Smsc();
  void init();
  void run();
protected:
  smsc::core::threads::ThreadPool tp;
  smsc::util::config::Manager* cfgman;
  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;

};

};//system
};//smsc


#endif
