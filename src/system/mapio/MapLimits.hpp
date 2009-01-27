#ifndef __SMSC_SYSTEM_MAPIO_MAPLIMITS__
#define __SMSC_SYSTEM_MAPIO_MAPLIMITS__

#include <string>
#include <set>
#include "core/synchronization/Mutex.hpp"

namespace smsc{
namespace system{
namespace mapio{
  
namespace sync=smsc::core::synchronization;

class MapLimits{
public:
  MapLimits()
  {
    limitIn=0;
    limitInSRI=0;
    limitUSSD=0;
    limitOut=0;
    limitNIUSSD=0;
  }


  int getLimitIn()
  {
    return limitIn;
  }
  int getLimitInSRI()
  {
    return limitInSRI;
  }
  int getLimitUSSD()
  {
    return limitUSSD;
  }
  int getLimitOut()
  {
    return limitOut;
  }
  int getLimitNIUSSD()
  {
    return limitNIUSSD;
  }
  
  bool isNoSRIUssd(const std::string& ussd)
  {
    sync::MutexGuard mg(mtx);
    std::string::size_type pos=ussd.find(':');
    if(pos!=std::string::npos)
    {
      return noSriUssd.find(ussd.substr(pos+1))!=noSriUssd.end();
    }
    return noSriUssd.find(ussd)!=noSriUssd.end();
  }

  static void Init(const char* fn);
  static void Shutdown()
  {
    delete instance;
    instance=0;
  }
  static MapLimits& getInstance()
  {
    return *instance;
  }

  void Reinit();
protected:
  static MapLimits* instance;

  std::string configFilename;
  
  std::set<std::string> noSriUssd;
  sync::Mutex mtx;

  int limitIn;
  int limitInSRI;
  int limitUSSD;
  int limitOut;
  int limitNIUSSD;
};

}
}
}

#endif

