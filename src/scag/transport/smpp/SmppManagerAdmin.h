#ifndef __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN_H__
#define __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN_H__

#include "sms/sms_const.h"
#include "SmppTypes.h"

namespace scag{
namespace transport{
namespace smpp{

struct SmppEntityInfo{
  SmppEntityType type;
  char systemId[MAX_SMESYSID_TYPE_LENGTH+1];
  char password[MAX_SMEPASSWD_TYPE_LENGTH+1];

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
