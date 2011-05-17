#ifndef __SMSC_CLUSTER_CONTROLLER_PROTOCOL_CONTROLLERSMSCPROTOCOL_HPP__
#define __SMSC_CLUSTER_CONTROLLER_PROTOCOL_CONTROLLERSMSCPROTOCOL_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "ControllerHandler.hpp"
#include "messages/LockConfig.hpp"
#include "messages/UnlockConfig.hpp"
#include "messages/GetSmscConfigsStateResp.hpp"
#include "messages/RegisterAsSmsc.hpp"
#include "messages/ApplyRoutesResp.hpp"
#include "messages/ApplyRescheduleResp.hpp"
#include "messages/ApplyLocaleResourceResp.hpp"
#include "messages/ApplyTimeZonesResp.hpp"
#include "messages/ApplyFraudControlResp.hpp"
#include "messages/ApplyMapLimitsResp.hpp"
#include "messages/ApplyNetProfilesResp.hpp"
#include "messages/ApplySnmpResp.hpp"
#include "messages/UpdateProfileResp.hpp"
#include "messages/DeleteProfileResp.hpp"
#include "messages/CancelSmsResp.hpp"
#include "messages/MscAddResp.hpp"
#include "messages/MscRemoveResp.hpp"
#include "messages/SmeAddResp.hpp"
#include "messages/SmeUpdateResp.hpp"
#include "messages/SmeRemoveResp.hpp"
#include "messages/SmeStatusResp.hpp"
#include "messages/SmeDisconnectResp.hpp"
#include "messages/LoggerGetCategoriesResp.hpp"
#include "messages/LoggerSetCategoriesResp.hpp"
#include "messages/AclRemoveResp.hpp"
#include "messages/AclCreateResp.hpp"
#include "messages/AclUpdateResp.hpp"
#include "messages/AclRemoveAddressesResp.hpp"
#include "messages/AclAddAddressesResp.hpp"
#include "messages/CgmAddGroupResp.hpp"
#include "messages/CgmDeleteGroupResp.hpp"
#include "messages/CgmAddAddrResp.hpp"
#include "messages/CgmDelAddrResp.hpp"
#include "messages/CgmAddAbonentResp.hpp"
#include "messages/CgmDelAbonentResp.hpp"
#include "messages/AliasAddResp.hpp"
#include "messages/AliasDelResp.hpp"
#include "messages/UpdateProfileAbnt.hpp"
#include "messages/GetServicesStatusResp.hpp"
#include "messages/DisconnectServiceResp.hpp"

namespace smsc {
namespace cluster {
namespace controller {
namespace protocol {

class ControllerSmscProtocol{
public:
  enum{
    tag_ApplyRoutes=1,
    tag_ApplyReschedule=2,
    tag_ApplyLocaleResource=3,
    tag_ApplyTimeZones=4,
    tag_ApplyFraudControl=5,
    tag_ApplyMapLimits=6,
    tag_ApplyNetProfiles=43,
    tag_ApplySnmp=7,
    tag_UpdateProfile=12,
    tag_DeleteProfile=14,
    tag_CancelSms=15,
    tag_MscAdd=16,
    tag_MscRemove=17,
    tag_SmeAdd=18,
    tag_SmeUpdate=19,
    tag_SmeRemove=20,
    tag_SmeStatus=21,
    tag_SmeDisconnect=22,
    tag_LoggerGetCategories=23,
    tag_LoggerSetCategories=24,
    tag_AclRemove=27,
    tag_AclCreate=28,
    tag_AclUpdate=29,
    tag_AclRemoveAddresses=31,
    tag_AclAddAddresses=32,
    tag_CgmAddGroup=33,
    tag_CgmDeleteGroup=34,
    tag_CgmAddAddr=35,
    tag_CgmDelAddr=37,
    tag_CgmAddAbonent=38,
    tag_CgmDelAbonent=39,
    tag_AliasAdd=41,
    tag_AliasDel=42,
    tag_UpdateProfileAbntResp=1013,
    tag_GetServicesStatus=101,
    tag_DisconnectService=102,
    tag_LockConfig=201,
    tag_UnlockConfig=202,
    tag_GetSmscConfigsStateResp=1205,
    tag_RegisterAsSmsc=303,
    tag_ApplyRoutesResp=1001,
    tag_ApplyRescheduleResp=1002,
    tag_ApplyLocaleResourceResp=1003,
    tag_ApplyTimeZonesResp=1004,
    tag_ApplyFraudControlResp=1005,
    tag_ApplyMapLimitsResp=1006,
    tag_ApplyNetProfilesResp=1043,
    tag_ApplySnmpResp=1007,
    tag_UpdateProfileResp=1012,
    tag_DeleteProfileResp=1014,
    tag_CancelSmsResp=1015,
    tag_MscAddResp=1016,
    tag_MscRemoveResp=1017,
    tag_SmeAddResp=1018,
    tag_SmeUpdateResp=1019,
    tag_SmeRemoveResp=1020,
    tag_SmeStatusResp=1021,
    tag_SmeDisconnectResp=1022,
    tag_LoggerGetCategoriesResp=1023,
    tag_LoggerSetCategoriesResp=1024,
    tag_AclRemoveResp=1027,
    tag_AclCreateResp=1028,
    tag_AclUpdateResp=1029,
    tag_AclRemoveAddressesResp=1031,
    tag_AclAddAddressesResp=1032,
    tag_CgmAddGroupResp=1033,
    tag_CgmDeleteGroupResp=1034,
    tag_CgmAddAddrResp=1035,
    tag_CgmDelAddrResp=1037,
    tag_CgmAddAbonentResp=1038,
    tag_CgmDelAbonentResp=1039,
    tag_AliasAddResp=1041,
    tag_AliasDelResp=1042,
    tag_UpdateProfileAbnt=13,
    tag_GetServicesStatusResp=1101,
    tag_DisconnectServiceResp=1102,
    tag_LockConfigResp=1201,
    tag_GetSmscConfigsState=204
  };
 
  ControllerSmscProtocol():handler(0)
  {
  }

  void assignHandler(ControllerHandler* newHandler);
  void decodeAndHandleMessage(const char* buf,size_t sz);
  void decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss);

  template <class MsgType>
  void encodeMessage(const MsgType& msg,eyeline::protogen::framework::SerializerBuffer* ss)
  {
    ss->writeInt32(4+4+msg.template length<eyeline::protogen::framework::SerializerBuffer>());
    ss->writeInt32(msg.messageGetTag());
    ss->writeInt32(msg.messageGetSeqNum());
    msg.serialize(*ss);
  }

protected:
  ControllerHandler* handler;
};

}
}
}
}

#endif
