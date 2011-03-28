#ifndef __SMSC_SYSTEM_MAPIO_NETWORKS_HPP__
#define __SMSC_SYSTEM_MAPIO_NETWORKS_HPP__
#include <string>
#include "core/synchronization/Mutex.hpp"
#include "sms/sms.h"
#include "logger/Logger.h"
#include <list>
#include "core/buffers/XTree.hpp"

namespace smsc{
namespace system{
namespace mapio{

enum DestRefValue{
  drvDestAddr,
  drvDestIMSI,
  drvEmpty
};

enum AbonentStatusMethod{
  asmSRI4SM,
  asmATI
};

struct NetworkProfile{
  NetworkProfile():drv(drvDestAddr),asMethod(asmSRI4SM),niVlrMethod(asmSRI4SM){}
  std::string name;
  DestRefValue drv;
  AbonentStatusMethod asMethod;
  AbonentStatusMethod niVlrMethod;
};

class NetworkProfiles{
public:
  static void init(const char* configFileName);
  static void shutdown();
  static NetworkProfiles& getInstance()
  {
    return *instance;
  }
  void reload();
  const NetworkProfile& lookup(const smsc::sms::Address& addr);
protected:
  static NetworkProfiles* instance;

  ~NetworkProfiles()
  {
    Clear();
  }

  void Clear();

  smsc::logger::Logger* log;
  std::string configFile;
  smsc::core::synchronization::Mutex mtx;
  smsc::core::buffers::XTree<NetworkProfile*> profiles;
  typedef std::list<NetworkProfile*> NetworkList;
  NetworkList netList;
  NetworkProfile defaultProfile;
};

}
}
}

#endif
