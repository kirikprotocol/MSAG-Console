#ifndef __SMSC_CLUSTER_CONTROLLER_PROTOCOL_CONTROLLERSMSCPROTOCOL_HPP__
#define __SMSC_CLUSTER_CONTROLLER_PROTOCOL_CONTROLLERSMSCPROTOCOL_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "ControllerHandler.hpp"
#include "messages/MultipartMessageRequest.hpp"
#include "messages/ReplaceIfPresentRequest.hpp"
#include "messages/LockProfiler.hpp"
#include "messages/UnlockProfiler.hpp"
#include "messages/LockMscManager.hpp"
#include "messages/UnlockMscManager.hpp"
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
#include "messages/DlPrcListResp.hpp"
#include "messages/DlPrcAddResp.hpp"
#include "messages/DlPrcDeleteResp.hpp"
#include "messages/DlPrcGetResp.hpp"
#include "messages/DlPrcAlterResp.hpp"
#include "messages/DlMemAddResp.hpp"
#include "messages/DlMemDeleteResp.hpp"
#include "messages/DlMemGetResp.hpp"
#include "messages/DlSbmAddResp.hpp"
#include "messages/DlSbmDelResp.hpp"
#include "messages/DlSbmListResp.hpp"
#include "messages/DlAddResp.hpp"
#include "messages/DlDeleteResp.hpp"
#include "messages/DlGetResp.hpp"
#include "messages/DlListResp.hpp"
#include "messages/DlAlterResp.hpp"
#include "messages/DlCopyResp.hpp"
#include "messages/DlRenameResp.hpp"
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
    tag_DlPrcList=35,
    tag_DlPrcAdd=36,
    tag_DlPrcDelete=37,
    tag_DlPrcGet=38,
    tag_DlPrcAlter=39,
    tag_DlMemAdd=40,
    tag_DlMemDelete=41,
    tag_DlMemGet=42,
    tag_DlSbmAdd=43,
    tag_DlSbmDel=44,
    tag_DlSbmList=45,
    tag_DlAdd=46,
    tag_DlDelete=47,
    tag_DlGet=48,
    tag_DlList=49,
    tag_DlAlter=50,
    tag_DlCopy=51,
    tag_DlRename=52,
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
    tag_GetServicesStatus=101,
    tag_DisconnectService=102,
    tag_MultipartMessageRequest=201,
    tag_ReplaceIfPresentRequest=202,
    tag_LockProfiler=203,
    tag_UnlockProfiler=204,
    tag_LockMscManager=205,
    tag_UnlockMscManager=206,
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
    tag_DlPrcListResp=1035,
    tag_DlPrcAddResp=1036,
    tag_DlPrcDeleteResp=1037,
    tag_DlPrcGetResp=1038,
    tag_DlPrcAlterResp=1039,
    tag_DlMemAddResp=1040,
    tag_DlMemDeleteResp=1041,
    tag_DlMemGetResp=1042,
    tag_DlSbmAddResp=1043,
    tag_DlSbmDelResp=1044,
    tag_DlSbmListResp=1045,
    tag_DlAddResp=1046,
    tag_DlDeleteResp=1047,
    tag_DlGetResp=1048,
    tag_DlListResp=1049,
    tag_DlAlterResp=1050,
    tag_DlCopyResp=1051,
    tag_DlRenameResp=1052,
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
    tag_GetServicesStatusResp=1101,
    tag_DisconnectServiceResp=1101,
    tag_MultipartMessageRequestResp=1201,
    tag_ReplaceIfPresentRequestResp=1202,
    tag_LockProfilerResp=1203,
    tag_LockMscManagerResp=1205
  };
 
  ControllerSmscProtocol():handler(0)
  {
  }

  void assignHandler(ControllerHandler* newHandler);
  void decodeAndHandleMessage(const char* buf,size_t sz);
  void decodeAndHandleMessage(protogen::framework::SerializerBuffer& ss);

  void encodeMessage(const messages::MultipartMessageRequest& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ReplaceIfPresentRequest& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LockProfiler& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::UnlockProfiler& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LockMscManager& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::UnlockMscManager& msg,protogen::framework::SerializerBuffer* ss);
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
  void encodeMessage(const messages::DlPrcListResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlPrcAddResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlPrcDeleteResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlPrcGetResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlPrcAlterResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlMemAddResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlMemDeleteResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlMemGetResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlSbmAddResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlSbmDelResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlSbmListResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlAddResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlDeleteResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlGetResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlListResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlAlterResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlCopyResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlRenameResp& msg,protogen::framework::SerializerBuffer* ss);
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
  void encodeMessage(const messages::GetServicesStatusResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DisconnectServiceResp& msg,protogen::framework::SerializerBuffer* ss);
protected:
  ControllerHandler* handler;
};

}
}
}
}

#endif
