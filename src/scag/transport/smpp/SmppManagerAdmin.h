#ifndef __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN_H__
#define __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN_H__

#include "sms/sms_const.h"
#include "SmppTypes.h"
#include "core/buffers/FixedLengthString.hpp"
#include <strings.h>
#include <list>

namespace scag{
namespace transport{
namespace smpp{

namespace buf=smsc::core::buffers;


struct SmppEntityAdminInfo
{
  //SmppEntityType type;
  std::string systemId;
  std::string host;
  int  port;
  bool connected;

};

typedef std::list<SmppEntityAdminInfo> SmppEntityAdminInfoList;


struct SmppEntityInfo{
  SmppEntityType type;
  buf::FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> systemId;
  buf::FixedLengthString<smsc::sms::MAX_SMEPASSWD_TYPE_LENGTH+1> password;
  buf::FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> bindSystemId;
  buf::FixedLengthString<smsc::sms::MAX_SMEPASSWD_TYPE_LENGTH+1> bindPassword;
  buf::FixedLengthString<42> addressRange;
  buf::FixedLengthString<13> systemType;
  int timeOut;
  SmppBindType bindType;
  buf::FixedLengthString<32> host;
  int  port;
  buf::FixedLengthString<32> altHost;
  int  altPort;
  uint8_t uid;

  SmppEntityInfo()
  {
    type=etUnknown;
    timeOut=0;
    bindType=btNone;
    port=0;
    altPort=0;
  }
};

struct SmppManagerAdmin{
  virtual void addSmppEntity(const SmppEntityInfo& info)=0;
  virtual void updateSmppEntity(const SmppEntityInfo& info)=0;
  virtual void deleteSmppEntity(const char* sysId)=0;
  virtual SmppEntityAdminInfoList * getEntityAdminInfoList(SmppEntityType entType) = 0;
};

}//smpp
}//transport
}//scag


#endif
