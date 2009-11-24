#ifndef SMSC_MSCMAN_MSC_MANAGER
#define SMSC_MSCMAN_MSC_MANAGER

#include <vector>

#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "core/buffers/Hash.hpp"
#include "logger/Logger.h"

namespace smsc {
namespace mscman {

using smsc::logger::Logger;

namespace sync=smsc::core::synchronization;
namespace buf=smsc::core::buffers;

class MscManager
{
protected:

  sync::Mutex mtx;
  Logger *log;
  buf::Hash<bool> mscs;

  static MscManager* instance;

  MscManager();

  void Init();

public:

  static void startup();
  static void shutdown();

  static MscManager& getInstance();

  bool check(const std::string& msc);
  void add(const std::string& msc);
  void remove(const std::string& msc);

};

}//mscman
}//smsc

#endif //SMSC_MSCMAN_MSC_MANAGER
