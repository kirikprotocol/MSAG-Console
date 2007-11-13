#ifndef __SCAG_TRANSPORT_SMPP_SMSCCONNECTORADMIN_H__
#define __SCAG_TRANSPORT_SMPP_SMSCCONNECTORADMIN_H__

#include <string>
#include "util/Exception.hpp"

namespace scag{
namespace transport{
namespace smpp{

struct SmscConnectInfo{
  std::string regSysId;
  std::string sysId;
  std::string pass;
  std::string hosts[2];
  int ports[2];
  int lastIdx;
  time_t lastFailure;
  std::string addressRange;
  std::string systemType;
  bool enabled;

  SmscConnectInfo()
  {
    lastIdx=0;
    lastFailure=0;
  }
  const char* host()const
  {
    return hosts[lastIdx].c_str();
  }
  int port()const
  {
    return ports[lastIdx];
  }
  void reportFailure()
  {
    lastIdx=(lastIdx+1)&1;
    lastFailure=time(NULL);
  }
};

class SmscConnectAlreadyExistsException:public smsc::util::Exception{
public:
  SmscConnectAlreadyExistsException(const char* sysId):
    smsc::util::Exception("Smsc connection with sysId='%s' already exists",sysId)
  {
  }
};

struct SmscConnectorAdmin{
  virtual void addSmscConnect(const SmscConnectInfo& info)=0;
  virtual void deleteSmscConnect(const char* sysId)=0;
  virtual void updateSmscConnect(const SmscConnectInfo& info)=0;
  virtual void reportSmscDisconnect(const char* sysId)=0;
};

}//smpp
}//transport
}//scag


#endif
