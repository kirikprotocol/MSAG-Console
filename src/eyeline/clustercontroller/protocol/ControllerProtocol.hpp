#ifndef __EYELINE_CLUSTERCONTROLLER_PROTOCOL_CONTROLLERPROTOCOL_HPP__
#define __EYELINE_CLUSTERCONTROLLER_PROTOCOL_CONTROLLERPROTOCOL_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "ControllerProtocolHandler.hpp"
#include "messages/ApplyRoutes.hpp"
#include "messages/ApplyReschedule.hpp"
#include "messages/ApplyLocaleResource.hpp"
#include "messages/ApplyTimeZones.hpp"
#include "messages/ApplyFraudControl.hpp"
#include "messages/ApplyMapLimits.hpp"
#include "messages/ApplySnmp.hpp"
#include "messages/TraceRoute.hpp"
#include "messages/LoadRoutes.hpp"
#include "messages/LookupProfile.hpp"
#include "messages/LookupProfileEx.hpp"
#include "messages/UpdateProfile.hpp"
#include "messages/DeleteProfile.hpp"
#include "messages/CancelSms.hpp"
#include "messages/MscAdd.hpp"
#include "messages/MscRemove.hpp"
#include "messages/SmeAdd.hpp"
#include "messages/SmeUpdate.hpp"
#include "messages/SmeRemove.hpp"
#include "messages/SmeStatus.hpp"
#include "messages/SmeDisconnect.hpp"
#include "messages/LoggerGetCategories.hpp"
#include "messages/LoggerSetCategories.hpp"
#include "messages/AclGet.hpp"
#include "messages/AclList.hpp"
#include "messages/AclRemove.hpp"
#include "messages/AclCreate.hpp"
#include "messages/AclUpdate.hpp"
#include "messages/AclLookup.hpp"
#include "messages/AclRemoveAddresses.hpp"
#include "messages/AclAddAddresses.hpp"
#include "messages/CgmAddGroup.hpp"
#include "messages/CgmDeleteGroup.hpp"
#include "messages/CgmAddAddr.hpp"
#include "messages/CgmCheck.hpp"
#include "messages/CgmDelAddr.hpp"
#include "messages/CgmAddAbonent.hpp"
#include "messages/CgmDelAbonent.hpp"
#include "messages/CgmListAbonents.hpp"
#include "messages/AliasAdd.hpp"
#include "messages/AliasDel.hpp"
#include "messages/UpdateProfileAbntResp.hpp"
#include "messages/GetServicesStatus.hpp"
#include "messages/DisconnectService.hpp"
#include "messages/LockConfig.hpp"
#include "messages/UnlockConfig.hpp"
#include "messages/RegisterAsLoadBalancer.hpp"
#include "messages/RegisterAsWebapp.hpp"
#include "messages/RegisterAsSmsc.hpp"
#include "messages/ApplyRoutesResp.hpp"
#include "messages/ApplyRescheduleResp.hpp"
#include "messages/ApplyLocaleResourceResp.hpp"
#include "messages/ApplyTimeZonesResp.hpp"
#include "messages/ApplyFraudControlResp.hpp"
#include "messages/ApplyMapLimitsResp.hpp"
#include "messages/ApplySnmpResp.hpp"
#include "messages/TraceRouteResp.hpp"
#include "messages/LoadRoutesResp.hpp"
#include "messages/LookupProfileResp.hpp"
#include "messages/LookupProfileExResp.hpp"
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
#include "messages/AclGetResp.hpp"
#include "messages/AclListResp.hpp"
#include "messages/AclRemoveResp.hpp"
#include "messages/AclCreateResp.hpp"
#include "messages/AclUpdateResp.hpp"
#include "messages/AclLookupResp.hpp"
#include "messages/AclRemoveAddressesResp.hpp"
#include "messages/AclAddAddressesResp.hpp"
#include "messages/CgmAddGroupResp.hpp"
#include "messages/CgmDeleteGroupResp.hpp"
#include "messages/CgmAddAddrResp.hpp"
#include "messages/CgmCheckResp.hpp"
#include "messages/CgmDelAddrResp.hpp"
#include "messages/CgmAddAbonentResp.hpp"
#include "messages/CgmDelAbonentResp.hpp"
#include "messages/CgmListAbonentsResp.hpp"
#include "messages/AliasAddResp.hpp"
#include "messages/AliasDelResp.hpp"
#include "messages/UpdateProfileAbnt.hpp"
#include "messages/GetServicesStatusResp.hpp"
#include "messages/DisconnectServiceResp.hpp"
#include "messages/LockConfigResp.hpp"

namespace eyeline {
namespace clustercontroller {
namespace protocol {

class ControllerProtocol{
public:
  enum{
    tag_ApplyRoutes=1,
    tag_ApplyReschedule=2,
    tag_ApplyLocaleResource=3,
    tag_ApplyTimeZones=4,
    tag_ApplyFraudControl=5,
    tag_ApplyMapLimits=6,
    tag_ApplySnmp=7,
    tag_TraceRoute=8,
    tag_LoadRoutes=9,
    tag_LookupProfile=10,
    tag_LookupProfileEx=11,
    tag_UpdateProfile=12,
    tag_DeleteProfile=13,
    tag_CancelSms=14,
    tag_MscAdd=15,
    tag_MscRemove=16,
    tag_SmeAdd=20,
    tag_SmeUpdate=21,
    tag_SmeRemove=22,
    tag_SmeStatus=23,
    tag_SmeDisconnect=24,
    tag_LoggerGetCategories=25,
    tag_LoggerSetCategories=26,
    tag_AclGet=27,
    tag_AclList=28,
    tag_AclRemove=29,
    tag_AclCreate=30,
    tag_AclUpdate=31,
    tag_AclLookup=32,
    tag_AclRemoveAddresses=33,
    tag_AclAddAddresses=34,
    tag_CgmAddGroup=53,
    tag_CgmDeleteGroup=54,
    tag_CgmAddAddr=55,
    tag_CgmCheck=56,
    tag_CgmDelAddr=57,
    tag_CgmAddAbonent=58,
    tag_CgmDelAbonent=59,
    tag_CgmListAbonents=60,
    tag_AliasAdd=61,
    tag_AliasDel=62,
    tag_UpdateProfileAbntResp=63,
    tag_GetServicesStatus=101,
    tag_DisconnectService=102,
    tag_LockConfig=203,
    tag_UnlockConfig=204,
    tag_RegisterAsLoadBalancer=301,
    tag_RegisterAsWebapp=302,
    tag_RegisterAsSmsc=303,
    tag_ApplyRoutesResp=1001,
    tag_ApplyRescheduleResp=1002,
    tag_ApplyLocaleResourceResp=1003,
    tag_ApplyTimeZonesResp=1004,
    tag_ApplyFraudControlResp=1005,
    tag_ApplyMapLimitsResp=1006,
    tag_ApplySnmpResp=1007,
    tag_TraceRouteResp=1008,
    tag_LoadRoutesResp=1009,
    tag_LookupProfileResp=1010,
    tag_LookupProfileExResp=1011,
    tag_UpdateProfileResp=1012,
    tag_DeleteProfileResp=1013,
    tag_CancelSmsResp=1014,
    tag_MscAddResp=1015,
    tag_MscRemoveResp=1016,
    tag_SmeAddResp=1020,
    tag_SmeUpdateResp=1021,
    tag_SmeRemoveResp=1022,
    tag_SmeStatusResp=1023,
    tag_SmeDisconnectResp=1024,
    tag_LoggerGetCategoriesResp=1025,
    tag_LoggerSetCategoriesResp=1026,
    tag_AclGetResp=1027,
    tag_AclListResp=1028,
    tag_AclRemoveResp=1029,
    tag_AclCreateResp=1030,
    tag_AclUpdateResp=1031,
    tag_AclLookupResp=1032,
    tag_AclRemoveAddressesResp=1033,
    tag_AclAddAddressesResp=1034,
    tag_CgmAddGroupResp=1053,
    tag_CgmDeleteGroupResp=1054,
    tag_CgmAddAddrResp=1055,
    tag_CgmCheckResp=1056,
    tag_CgmDelAddrResp=1057,
    tag_CgmAddAbonentResp=1058,
    tag_CgmDelAbonentResp=1059,
    tag_CgmListAbonentsResp=1060,
    tag_AliasAddResp=1061,
    tag_AliasDelResp=1062,
    tag_UpdateProfileAbnt=1063,
    tag_GetServicesStatusResp=1101,
    tag_DisconnectServiceResp=1102,
    tag_LockConfigResp=1203
  };
 
  ControllerProtocol():handler(0)
  {
  }

  void assignHandler(ControllerProtocolHandler* newHandler);
  void decodeAndHandleMessage(const char* buf,size_t sz);
  void decodeAndHandleMessage(protogen::framework::SerializerBuffer& ss);

  void encodeMessage(const messages::ApplyRoutes& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyReschedule& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyLocaleResource& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyTimeZones& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyFraudControl& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyMapLimits& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplySnmp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::TraceRoute& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LoadRoutes& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LookupProfile& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LookupProfileEx& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::UpdateProfile& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DeleteProfile& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CancelSms& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MscAdd& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MscRemove& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeAdd& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeUpdate& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeRemove& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeStatus& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeDisconnect& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LoggerGetCategories& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LoggerSetCategories& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclGet& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclList& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclRemove& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclCreate& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclUpdate& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclLookup& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclRemoveAddresses& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclAddAddresses& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmAddGroup& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmDeleteGroup& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmAddAddr& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmCheck& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmDelAddr& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmAddAbonent& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmDelAbonent& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmListAbonents& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AliasAdd& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AliasDel& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::UpdateProfileAbntResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::GetServicesStatus& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DisconnectService& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LockConfig& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::UnlockConfig& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::RegisterAsLoadBalancer& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::RegisterAsWebapp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::RegisterAsSmsc& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyRoutesResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyRescheduleResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyLocaleResourceResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyTimeZonesResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyFraudControlResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplyMapLimitsResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ApplySnmpResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::TraceRouteResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LoadRoutesResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LookupProfileResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LookupProfileExResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::UpdateProfileResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DeleteProfileResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CancelSmsResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MscAddResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MscRemoveResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeAddResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeUpdateResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeRemoveResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeStatusResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::SmeDisconnectResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LoggerGetCategoriesResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LoggerSetCategoriesResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclGetResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclListResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclRemoveResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclCreateResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclUpdateResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclLookupResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclRemoveAddressesResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AclAddAddressesResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmAddGroupResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmDeleteGroupResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmAddAddrResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmCheckResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmDelAddrResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmAddAbonentResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmDelAbonentResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::CgmListAbonentsResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AliasAddResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::AliasDelResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::UpdateProfileAbnt& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::GetServicesStatusResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DisconnectServiceResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LockConfigResp& msg,protogen::framework::SerializerBuffer* ss);
protected:
  ControllerProtocolHandler* handler;
};

}
}
}

#endif
