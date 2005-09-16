#ifndef __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN_H__
#define __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN_H__

#include "sms/sms_const.h"
#include "SmppTypes.h"

namespace scag{
namespace transport{
namespace smpp{

struct SmppEntityInfo{
  SmppEntityType type;
  char systemId[smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1];
  char password[smsc::sms::MAX_SMEPASSWD_TYPE_LENGTH+1];
  int timeOut;
  SmppBindType bindType;
  char host[32];
  int  port;
  char altHost[32];
  int  altPort;

  SmppEntityInfo()
  {
    type=etUnknown;
    systemId[0]=0;
    password[0]=0;
    timeOut=0;
    bindType=btNone;
    host[0]=0;
    port=0;
    altHost[0]=0;
    altPort=0;
  }
};

struct SmppManagerAdmin{
  virtual void addSmppEntity(const SmppEntityInfo& info)=0;
  virtual void updateSmppEntity(const SmppEntityInfo& info)=0;
  virtual void deleteSmppEntity(const char* sysId)=0;
};

}//smpp
}//transport
}//scag


#endif
