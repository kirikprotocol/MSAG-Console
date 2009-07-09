#ifndef __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN2_H__

#include "sms/sms_const.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "core/buffers/FixedLengthString.hpp"
#include <strings.h>
#include <list>

namespace scag2 {
namespace transport {
namespace smpp {

using namespace smsc::core::buffers;

struct SmppEntityAdminInfo
{
  //SmppEntityType type;
  std::string systemId;
  SmppBindType bindType;
  std::string host;
  int  port;
  bool connected;

};

typedef std::list<SmppEntityAdminInfo> SmppEntityAdminInfoList;


struct SmppEntityInfo {
  SmppEntityType type;
  FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> systemId;
  FixedLengthString<smsc::sms::MAX_SMEPASSWD_TYPE_LENGTH+1> password;
  FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> bindSystemId;
  FixedLengthString<smsc::sms::MAX_SMEPASSWD_TYPE_LENGTH+1> bindPassword;
  FixedLengthString<42> addressRange;
  FixedLengthString<13> systemType;
  FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> metaGroupId;
  int timeOut;
  SmppBindType bindType;
  FixedLengthString<32> host;
  int  port;
  FixedLengthString<32> altHost;
  int  altPort;
  uint8_t uid;
  int sendLimit;
  int inQueueLimit;
  int outQueueLimit;
  bool enabled;
  bool snmpTracking;    // watch on this sme via snmp

  SmppEntityInfo()
  {
    type=etUnknown;
    timeOut=0;
    bindType=btNone;
    port=0;
    altPort=0;
    sendLimit=0;
    inQueueLimit=0;
    outQueueLimit=0;
    enabled=false;
    snmpTracking=true;
  }
};

enum BalancingPolicy{
  bpRoundRobin,bpRandom
};
enum MetaEntityType{
  mtMetaService,mtMetaSmsc
};
struct MetaEntityInfo{
  MetaEntityInfo()
  {
    type=mtMetaService;
    policy=bpRoundRobin;
    persistanceEnabled=true;
  }
  FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> systemId;
  MetaEntityType type;
  BalancingPolicy policy;
  bool persistanceEnabled;
};

struct SmppManagerAdmin{
  virtual void addSmppEntity(const SmppEntityInfo& info)=0;
  virtual void updateSmppEntity(const SmppEntityInfo& info)=0;
  virtual void disconnectSmppEntity(const char* sysId)=0;
  virtual void deleteSmppEntity(const char* sysId)=0;
  virtual void ReloadRoutes()=0;
  virtual SmppEntityAdminInfoList * getEntityAdminInfoList(SmppEntityType entType) = 0;

  virtual bool LoadEntityFromConfig(SmppEntityInfo& info,const char* sysId,SmppEntityType et)=0;
  virtual bool LoadMetaEntityFromConfig(MetaEntityInfo& info,const char* sysId)=0;

  virtual void addMetaEntity(MetaEntityInfo info)=0;
  virtual void updateMetaEntity(MetaEntityInfo info)=0;
  virtual void deleteMetaEntity(const char* id)=0;

  virtual void addMetaEndPoint(const char* metaId,const char* sysId)=0;
  virtual void removeMetaEndPoint(const char* metaId,const char* sysId)=0;
};

}//smpp
}//transport
}//scag


#endif
