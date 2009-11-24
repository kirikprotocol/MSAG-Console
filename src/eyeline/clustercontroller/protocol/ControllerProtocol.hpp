#ifndef __EYELINE_CLUSTERCONTROLLER_PROTOCOL_CONTROLLERPROTOCOL_HPP__
#define __EYELINE_CLUSTERCONTROLLER_PROTOCOL_CONTROLLERPROTOCOL_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "ControllerProtocolHandler.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyRoutes.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyReschedule.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyLocaleResource.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyTimeZones.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyFraudControl.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyMapLimits.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplySnmp.hpp"
#include "eyeline/clustercontroller/protocol/messages/TraceRoute.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoadRoutes.hpp"
#include "eyeline/clustercontroller/protocol/messages/LookupProfile.hpp"
#include "eyeline/clustercontroller/protocol/messages/LookupProfileEx.hpp"
#include "eyeline/clustercontroller/protocol/messages/UpdateProfile.hpp"
#include "eyeline/clustercontroller/protocol/messages/DeleteProfile.hpp"
#include "eyeline/clustercontroller/protocol/messages/CancelSms.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscRegister.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscUnregister.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscBlock.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscClear.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscList.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeAdd.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeUpdate.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeRemove.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeStatus.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeDisconnect.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoggerGetCategories.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoggerSetCategories.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclGet.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclList.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclRemove.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclCreate.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclUpdate.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclLookup.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclRemoveAddresses.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclAddAddresses.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcList.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcAdd.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcDelete.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcGet.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcAlter.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemAdd.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemDelete.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemGet.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmAdd.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmDel.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmList.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlAdd.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlDelete.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlGet.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlList.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlAlter.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlCopy.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlRename.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddGroup.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDeleteGroup.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddAddr.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmCheck.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDelAddr.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddAbonent.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDelAbonent.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmListAbonents.hpp"
#include "eyeline/clustercontroller/protocol/messages/AliasAdd.hpp"
#include "eyeline/clustercontroller/protocol/messages/AliasDel.hpp"
#include "eyeline/clustercontroller/protocol/messages/GetServicesStatus.hpp"
#include "eyeline/clustercontroller/protocol/messages/DisconnectService.hpp"
#include "eyeline/clustercontroller/protocol/messages/MultipartMessageRequestResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ReplaceIfPresentRequestResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LockProfilerResp.hpp"

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
    tag_MscRegister=15,
    tag_MscUnregister=16,
    tag_MscBlock=17,
    tag_MscClear=18,
    tag_MscList=19,
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
    tag_MscRegisterResp=1015,
    tag_MscUnregisterResp=1016,
    tag_MscBlockResp=1017,
    tag_MscClearResp=1018,
    tag_MscListResp=1019,
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
    tag_DisconnectServiceResp=1102,
    tag_MultipartMessageRequestResp=1201,
    tag_ReplaceIfPresentRequestResp=1202,
    tag_LockProfilerResp=1203
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
  void encodeMessage(const messages::MscRegister& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MscUnregister& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MscBlock& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MscClear& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MscList& msg,protogen::framework::SerializerBuffer* ss);
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
  void encodeMessage(const messages::DlPrcList& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlPrcAdd& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlPrcDelete& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlPrcGet& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlPrcAlter& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlMemAdd& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlMemDelete& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlMemGet& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlSbmAdd& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlSbmDel& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlSbmList& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlAdd& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlDelete& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlGet& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlList& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlAlter& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlCopy& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DlRename& msg,protogen::framework::SerializerBuffer* ss);
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
  void encodeMessage(const messages::GetServicesStatus& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DisconnectService& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::MultipartMessageRequestResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::ReplaceIfPresentRequestResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::LockProfilerResp& msg,protogen::framework::SerializerBuffer* ss);
protected:
  ControllerProtocolHandler* handler;
};

}
}
}

#endif
