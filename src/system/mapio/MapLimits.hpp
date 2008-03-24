#ifndef __SMSC_SYSTEM_MAPIO_MAPLIMITS__
#define __SMSC_SYSTEM_MAPIO_MAPLIMITS__

#include <string>

namespace smsc{
namespace system{
namespace mapio{

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

