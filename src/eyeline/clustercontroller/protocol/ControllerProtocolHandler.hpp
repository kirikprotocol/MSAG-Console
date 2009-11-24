#ifndef __EYELINE_CLUSTERCONTROLLER_PROTOCOL_CONTROLLERPROTOCOLHANDLER_HPP__
#define __EYELINE_CLUSTERCONTROLLER_PROTOCOL_CONTROLLERPROTOCOLHANDLER_HPP__ 1
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
#include "eyeline/clustercontroller/protocol/messages/MultipartMessageRequest.hpp"
#include "eyeline/clustercontroller/protocol/messages/ReplaceIfPresentRequest.hpp"
#include "eyeline/clustercontroller/protocol/messages/LockProfiler.hpp"
#include "eyeline/clustercontroller/protocol/messages/UnlockProfiler.hpp"
#include "eyeline/clustercontroller/protocol/messages/RegisterAsLoadBalancer.hpp"
#include "eyeline/clustercontroller/protocol/messages/RegisterAsWebapp.hpp"
#include "eyeline/clustercontroller/protocol/messages/RegisterAsSmsc.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyRoutesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyRescheduleResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyLocaleResourceResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyTimeZonesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyFraudControlResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyMapLimitsResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplySnmpResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/TraceRouteResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoadRoutesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LookupProfileResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LookupProfileExResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/UpdateProfileResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DeleteProfileResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CancelSmsResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscRegisterResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscUnregisterResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscBlockResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscClearResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeUpdateResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeRemoveResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeStatusResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeDisconnectResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoggerGetCategoriesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoggerSetCategoriesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclGetResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclRemoveResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclCreateResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclUpdateResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclLookupResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclRemoveAddressesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclAddAddressesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcDeleteResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcGetResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcAlterResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemDeleteResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemGetResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmDelResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlDeleteResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlGetResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlAlterResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlCopyResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlRenameResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddGroupResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDeleteGroupResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddAddrResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmCheckResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDelAddrResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddAbonentResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDelAbonentResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmListAbonentsResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AliasAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AliasDelResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/GetServicesStatusResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DisconnectServiceResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MultipartMessageRequestResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ReplaceIfPresentRequestResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LockProfilerResp.hpp"

namespace eyeline {
namespace clustercontroller {
namespace protocol {
class ControllerProtocolHandler{
public:
  ControllerProtocolHandler(int argConnId):connId(argConnId)
  {

  }
  void handle(const messages::MultipartMessageRequest& msg);
  void handle(const messages::ReplaceIfPresentRequest& msg);
  void handle(const messages::LockProfiler& msg);
  void handle(const messages::UnlockProfiler& msg);
  void handle(const messages::RegisterAsLoadBalancer& msg);
  void handle(const messages::RegisterAsWebapp& msg);
  void handle(const messages::RegisterAsSmsc& msg);
  void handle(const messages::ApplyRoutesResp& msg);
  void handle(const messages::ApplyRescheduleResp& msg);
  void handle(const messages::ApplyLocaleResourceResp& msg);
  void handle(const messages::ApplyTimeZonesResp& msg);
  void handle(const messages::ApplyFraudControlResp& msg);
  void handle(const messages::ApplyMapLimitsResp& msg);
  void handle(const messages::ApplySnmpResp& msg);
  void handle(const messages::TraceRouteResp& msg);
  void handle(const messages::LoadRoutesResp& msg);
  void handle(const messages::LookupProfileResp& msg);
  void handle(const messages::LookupProfileExResp& msg);
  void handle(const messages::UpdateProfileResp& msg);
  void handle(const messages::DeleteProfileResp& msg);
  void handle(const messages::CancelSmsResp& msg);
  void handle(const messages::MscRegisterResp& msg);
  void handle(const messages::MscUnregisterResp& msg);
  void handle(const messages::MscBlockResp& msg);
  void handle(const messages::MscClearResp& msg);
  void handle(const messages::MscListResp& msg);
  void handle(const messages::SmeAddResp& msg);
  void handle(const messages::SmeUpdateResp& msg);
  void handle(const messages::SmeRemoveResp& msg);
  void handle(const messages::SmeStatusResp& msg);
  void handle(const messages::SmeDisconnectResp& msg);
  void handle(const messages::LoggerGetCategoriesResp& msg);
  void handle(const messages::LoggerSetCategoriesResp& msg);
  void handle(const messages::AclGetResp& msg);
  void handle(const messages::AclListResp& msg);
  void handle(const messages::AclRemoveResp& msg);
  void handle(const messages::AclCreateResp& msg);
  void handle(const messages::AclUpdateResp& msg);
  void handle(const messages::AclLookupResp& msg);
  void handle(const messages::AclRemoveAddressesResp& msg);
  void handle(const messages::AclAddAddressesResp& msg);
  void handle(const messages::DlPrcListResp& msg);
  void handle(const messages::DlPrcAddResp& msg);
  void handle(const messages::DlPrcDeleteResp& msg);
  void handle(const messages::DlPrcGetResp& msg);
  void handle(const messages::DlPrcAlterResp& msg);
  void handle(const messages::DlMemAddResp& msg);
  void handle(const messages::DlMemDeleteResp& msg);
  void handle(const messages::DlMemGetResp& msg);
  void handle(const messages::DlSbmAddResp& msg);
  void handle(const messages::DlSbmDelResp& msg);
  void handle(const messages::DlSbmListResp& msg);
  void handle(const messages::DlAddResp& msg);
  void handle(const messages::DlDeleteResp& msg);
  void handle(const messages::DlGetResp& msg);
  void handle(const messages::DlListResp& msg);
  void handle(const messages::DlAlterResp& msg);
  void handle(const messages::DlCopyResp& msg);
  void handle(const messages::DlRenameResp& msg);
  void handle(const messages::CgmAddGroupResp& msg);
  void handle(const messages::CgmDeleteGroupResp& msg);
  void handle(const messages::CgmAddAddrResp& msg);
  void handle(const messages::CgmCheckResp& msg);
  void handle(const messages::CgmDelAddrResp& msg);
  void handle(const messages::CgmAddAbonentResp& msg);
  void handle(const messages::CgmDelAbonentResp& msg);
  void handle(const messages::CgmListAbonentsResp& msg);
  void handle(const messages::AliasAddResp& msg);
  void handle(const messages::AliasDelResp& msg);
  void handle(const messages::GetServicesStatusResp& msg);
  void handle(const messages::DisconnectServiceResp& msg);
protected:
  int connId;
};


}
}
}

#endif
