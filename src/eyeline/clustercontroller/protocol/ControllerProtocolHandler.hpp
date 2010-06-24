#ifndef __EYELINE_CLUSTERCONTROLLER_PROTOCOL_CONTROLLERPROTOCOLHANDLER_HPP__
#define __EYELINE_CLUSTERCONTROLLER_PROTOCOL_CONTROLLERPROTOCOLHANDLER_HPP__ 1
#include "messages/ApplyRoutes.hpp"
#include "messages/ApplyReschedule.hpp"
#include "messages/ApplyLocaleResource.hpp"
#include "messages/ApplyTimeZones.hpp"
#include "messages/ApplyFraudControl.hpp"
#include "messages/ApplyMapLimits.hpp"
#include "messages/ApplySnmp.hpp"
#include "messages/TraceRoute.hpp"
#include "messages/LoadTestRoutes.hpp"
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
#include "messages/GetServicesStatusResp.hpp"
#include "messages/DisconnectServiceResp.hpp"
#include "messages/LockConfigResp.hpp"
#include "messages/UpdateProfileAbnt.hpp"
#include "messages/UpdateProfileAbntResp.hpp"
#include "logger/Logger.h"
#include "messages/GetConfigsState.hpp"
#include "messages/GetSmscConfigsStateResp.hpp"

namespace eyeline {
namespace clustercontroller {
namespace protocol {
class ControllerProtocolHandler{
public:
  ControllerProtocolHandler(int argConnId,smsc::logger::Logger* argLog):connId(argConnId),log(argLog)
  {

  }
  void handle(messages::ApplyRoutes& msg);
  void handle(messages::ApplyReschedule& msg);
  void handle(messages::ApplyLocaleResource& msg);
  void handle(messages::ApplyTimeZones& msg);
  void handle(messages::ApplyFraudControl& msg);
  void handle(messages::ApplyMapLimits& msg);
  void handle(messages::ApplySnmp& msg);
  void handle(const messages::TraceRoute& msg);
  void handle(const messages::LoadTestRoutes& msg);
  void handle(const messages::LookupProfile& msg);
  void handle(const messages::LookupProfileEx& msg);
  void handle(messages::UpdateProfile& msg);
  void handle(messages::DeleteProfile& msg);
  void handle(messages::CancelSms& msg);
  void handle(messages::MscAdd& msg);
  void handle(messages::MscRemove& msg);
  void handle(messages::SmeAdd& msg);
  void handle(messages::SmeUpdate& msg);
  void handle(messages::SmeRemove& msg);
  void handle(messages::SmeStatus& msg);
  void handle(messages::SmeDisconnect& msg);
  void handle(messages::LoggerGetCategories& msg);
  void handle(messages::LoggerSetCategories& msg);
  void handle(const messages::AclGet& msg);
  void handle(const messages::AclList& msg);
  void handle(messages::AclRemove& msg);
  void handle(messages::AclCreate& msg);
  void handle(messages::AclUpdate& msg);
  void handle(const messages::AclLookup& msg);
  void handle(messages::AclRemoveAddresses& msg);
  void handle(messages::AclAddAddresses& msg);
  void handle(messages::CgmAddGroup& msg);
  void handle(messages::CgmDeleteGroup& msg);
  void handle(messages::CgmAddAddr& msg);
  void handle(const messages::CgmCheck& msg);
  void handle(messages::CgmDelAddr& msg);
  void handle(messages::CgmAddAbonent& msg);
  void handle(messages::CgmDelAbonent& msg);
  void handle(const messages::CgmListAbonents& msg);
  void handle(messages::AliasAdd& msg);
  void handle(messages::AliasDel& msg);
  void handle(const messages::GetServicesStatus& msg);
  void handle(const messages::DisconnectService& msg);
  void handle(const messages::LockConfig& msg);
  void handle(const messages::GetConfigsState& msg);
  void handle(const messages::UnlockConfig& msg);
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
  void handle(const messages::UpdateProfileResp& msg);
  void handle(const messages::DeleteProfileResp& msg);
  void handle(const messages::CancelSmsResp& msg);
  void handle(const messages::MscAddResp& msg);
  void handle(const messages::MscRemoveResp& msg);
  void handle(const messages::SmeAddResp& msg);
  void handle(const messages::SmeUpdateResp& msg);
  void handle(const messages::SmeRemoveResp& msg);
  void handle(const messages::SmeStatusResp& msg);
  void handle(const messages::SmeDisconnectResp& msg);
  void handle(messages::LoggerGetCategoriesResp& msg);
  void handle(messages::LoggerSetCategoriesResp& msg);
  void handle(const messages::AclRemoveResp& msg);
  void handle(const messages::AclCreateResp& msg);
  void handle(const messages::AclUpdateResp& msg);
  void handle(const messages::AclRemoveAddressesResp& msg);
  void handle(const messages::AclAddAddressesResp& msg);
  void handle(const messages::CgmAddGroupResp& msg);
  void handle(const messages::CgmDeleteGroupResp& msg);
  void handle(const messages::CgmAddAddrResp& msg);
  void handle(const messages::CgmDelAddrResp& msg);
  void handle(const messages::CgmAddAbonentResp& msg);
  void handle(const messages::CgmDelAbonentResp& msg);
  void handle(const messages::AliasAddResp& msg);
  void handle(const messages::AliasDelResp& msg);
  void handle(const messages::GetServicesStatusResp& msg);
  void handle(const messages::DisconnectServiceResp& msg);
  //void handle(const messages::LockConfigResp& msg);
  void handle(const messages::UpdateProfileAbnt& msg);
  void handle(const messages::GetSmscConfigsStateResp& msg);
protected:
  int connId;
  smsc::logger::Logger* log;
  template <class MSG_T,class MSG_RESP_T>
  void prepareResp(MSG_T& msg,MSG_RESP_T& respMsg,uint32_t status)
  {
    respMsg.setSeqNum(msg.getSeqNum());
    messages::Response resp;
    resp.setStatus(status);
    respMsg.setResp(resp);
  }
};


}
}
}

#endif
