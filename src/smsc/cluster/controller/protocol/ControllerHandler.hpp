#ifndef __SMSC_CLUSTER_CONTROLLER_PROTOCOL_CONTROLLERSMSCPROTOCOLHANDLER_HPP__
#define __SMSC_CLUSTER_CONTROLLER_PROTOCOL_CONTROLLERSMSCPROTOCOLHANDLER_HPP__ 1
#include "messages/ApplyRoutes.hpp"
#include "messages/ApplyReschedule.hpp"
#include "messages/ApplyLocaleResource.hpp"
#include "messages/ApplyTimeZones.hpp"
#include "messages/ApplyFraudControl.hpp"
#include "messages/ApplyMapLimits.hpp"
#include "messages/ApplySnmp.hpp"
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
/*
#include "messages/DlPrcList.hpp"
#include "messages/DlPrcAdd.hpp"
#include "messages/DlPrcDelete.hpp"
#include "messages/DlPrcGet.hpp"
#include "messages/DlPrcAlter.hpp"
#include "messages/DlMemAdd.hpp"
#include "messages/DlMemDelete.hpp"
#include "messages/DlMemGet.hpp"
#include "messages/DlSbmAdd.hpp"
#include "messages/DlSbmDel.hpp"
#include "messages/DlSbmList.hpp"
#include "messages/DlAdd.hpp"
#include "messages/DlDelete.hpp"
#include "messages/DlGet.hpp"
#include "messages/DlList.hpp"
#include "messages/DlAlter.hpp"
#include "messages/DlCopy.hpp"
#include "messages/DlRename.hpp"
*/
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
#include "messages/LockConfigResp.hpp"
#include "messages/UnlockConfig.hpp"
//#include "messages/RegisterAsLoadBalancer.hpp"
//#include "messages/RegisterAsWebapp.hpp"
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
#include "messages/AclGetResp.hpp"
#include "messages/AclListResp.hpp"
#include "messages/AclRemoveResp.hpp"
#include "messages/AclCreateResp.hpp"
#include "messages/AclUpdateResp.hpp"
#include "messages/AclLookupResp.hpp"
#include "messages/AclRemoveAddressesResp.hpp"
#include "messages/AclAddAddressesResp.hpp"
/*
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
*/
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
#include "logger/Logger.h"
#include "messages/UpdateProfileAbntResp.hpp"
#include "smsc/cluster/controller/protocol/messages/MultiResponse.hpp"
/*
#include "messages/DlMemAddAbntResp.hpp"
#include "messages/DlMemDeleteAbntResp.hpp"
#include "messages/DlSbmAddAbntResp.hpp"
#include "messages/DlSbmDelAbntResp.hpp"
#include "messages/DlAddAbntResp.hpp"
#include "messages/DlDeleteAbntResp.hpp"
#include "messages/DlCopyAbntResp.hpp"
#include "messages/DlRenameAbntResp.hpp"
*/

namespace smsc {
namespace cluster {
namespace controller {
namespace protocol {
class ControllerHandler{
public:
  ControllerHandler():log(smsc::logger::Logger::getInstance("cp.hnd")),isTempRouterLoaded(false)
  {

  }

  void Init();

  void handle(const messages::ApplyRoutes& msg);
  void handle(const messages::ApplyReschedule& msg);
  void handle(const messages::ApplyLocaleResource& msg);
  void handle(const messages::ApplyTimeZones& msg);
  void handle(const messages::ApplyFraudControl& msg);
  void handle(const messages::ApplyMapLimits& msg);
  void handle(const messages::ApplySnmp& msg);
  /*void handle(const messages::TraceRoute& msg);
  void handle(const messages::LoadRoutes& msg);
  void handle(const messages::LookupProfile& msg);
  void handle(const messages::LookupProfileEx& msg);*/
  void handle(const messages::UpdateProfile& msg);
  void handle(const messages::DeleteProfile& msg);
  void handle(const messages::CancelSms& msg);
  void handle(const messages::MscAdd& msg);
  void handle(const messages::MscRemove& msg);
  void handle(const messages::SmeAdd& msg);
  void handle(const messages::SmeUpdate& msg);
  void handle(const messages::SmeRemove& msg);
  void handle(const messages::SmeStatus& msg);
  void handle(const messages::SmeDisconnect& msg);
  void handle(const messages::LoggerGetCategories& msg);
  void handle(const messages::LoggerSetCategories& msg);
  void handle(const messages::AclGet& msg);
  void handle(const messages::AclList& msg);
  void handle(const messages::AclRemove& msg);
  void handle(const messages::AclCreate& msg);
  void handle(const messages::AclUpdate& msg);
  void handle(const messages::AclLookup& msg);
  void handle(const messages::AclRemoveAddresses& msg);
  void handle(const messages::AclAddAddresses& msg);
  /*
  void handle(const messages::DlPrcList& msg);
  void handle(const messages::DlPrcAdd& msg);
  void handle(const messages::DlPrcDelete& msg);
  void handle(const messages::DlPrcGet& msg);
  void handle(const messages::DlPrcAlter& msg);
  void handle(const messages::DlMemAdd& msg);
  void handle(const messages::DlMemDelete& msg);
  void handle(const messages::DlMemGet& msg);
  void handle(const messages::DlSbmAdd& msg);
  void handle(const messages::DlSbmDel& msg);
  void handle(const messages::DlSbmList& msg);
  void handle(const messages::DlAdd& msg);
  void handle(const messages::DlDelete& msg);
  void handle(const messages::DlGet& msg);
  void handle(const messages::DlList& msg);
  void handle(const messages::DlAlter& msg);
  void handle(const messages::DlCopy& msg);
  void handle(const messages::DlRename& msg);
  */
  void handle(const messages::CgmAddGroup& msg);
  void handle(const messages::CgmDeleteGroup& msg);
  void handle(const messages::CgmAddAddr& msg);
  void handle(const messages::CgmCheck& msg);
  void handle(const messages::CgmDelAddr& msg);
  void handle(const messages::CgmAddAbonent& msg);
  void handle(const messages::CgmDelAbonent& msg);
  void handle(const messages::CgmListAbonents& msg);
  void handle(const messages::AliasAdd& msg);
  void handle(const messages::AliasDel& msg);
  void handle(const messages::GetServicesStatus& msg);
  void handle(const messages::DisconnectService& msg);
  //void handle(const messages::MultipartMessageRequestResp& msg);
  //void handle(const messages::ReplaceIfPresentRequestResp& msg);
  void handle(const messages::LockConfigResp& msg);
  void handle(const messages::UpdateProfileAbntResp& msg);
/*
  void handle(const messages::DlMemAddAbntResp& msg);
  void handle(const messages::DlMemDeleteAbntResp& msg);
  void handle(const messages::DlSbmAddAbntResp& msg);
  void handle(const messages::DlSbmDelAbntResp& msg);
  void handle(const messages::DlAddAbntResp& msg);
  void handle(const messages::DlDeleteAbntResp& msg);
  void handle(const messages::DlCopyAbntResp& msg);
  void handle(const messages::DlRenameAbntResp& msg);
  */

protected:
  template <class MSG_T,class MSG_RESP_T>
  void prepareResp(MSG_T& msg,MSG_RESP_T& respMsg,uint32_t status)
  {
    respMsg.setSeqNum(msg.getSeqNum());
    messages::Response resp;
    resp.setStatus(status);
    respMsg.setResp(resp);
  }
  template <class MSG_T,class MSG_RESP_T>
  void prepareMultiResp(MSG_T& msg,MSG_RESP_T& respMsg,uint32_t status)
  {
    respMsg.setSeqNum(msg.getSeqNum());
    messages::MultiResponse resp;
    std::vector<int32_t> st;
    st.push_back(status);
    resp.setStatus(st);
    std::vector<int8_t> id;
    id.push_back(nodeIdx);
    resp.setIds(id);
    respMsg.setResp(resp);
  }
  smsc::logger::Logger* log;
  bool isTempRouterLoaded;
  int nodeIdx;
};


}
}
}
}

#endif
